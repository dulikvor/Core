#pragma once

#include <atomic>
#include <chrono>
#if defined(__linux)
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#endif
#include "Exception.h"

namespace core{
    class Mutex
    {
    private:
        enum Status : int
        {
            UNLOCK = 0,
            LOCK_SINGLE,
            LOCK_MANY
        };
    
        typedef std::chrono::high_resolution_clock 	__clock_t;
    
    public:
        Mutex(): m_word(UNLOCK) {}
        
        void lock()
        {
            int lockState = UNLOCK;
            if(std::atomic_compare_exchange_strong_explicit(&m_word, &lockState, static_cast<int>(LOCK_SINGLE),
                    std::memory_order_relaxed, std::memory_order_relaxed) == false)
            {
                if(lockState != LOCK_MANY)
                    lockState = std::atomic_exchange_explicit(&m_word, static_cast<int>(LOCK_MANY), std::memory_order_relaxed);
                while(lockState != UNLOCK)
                {

                #if defined(__linux)
                    syscall(SYS_futex, &m_word, FUTEX_WAIT, LOCK_MANY, nullptr, nullptr, 0);
                #else
                    throw Exception(__CORE_SOURCE, "wait is not being supported by current platform");
                #endif
                    lockState = std::atomic_exchange_explicit(&m_word, static_cast<int>(LOCK_MANY), std::memory_order_relaxed);
                }
            }
            std::atomic_thread_fence(std::memory_order_seq_cst); //linireazibility point
        }
    
        void unlock()
        {
            if(std::atomic_fetch_sub(&m_word, 1) == LOCK_SINGLE)
                return;
        
            m_word = UNLOCK;
#if defined(__linux)
            syscall(SYS_futex, &m_word, FUTEX_WAKE, 1, nullptr, nullptr, 0);
#else
            throw Exception(__CORE_SOURCE, "wake is not being supported by current platform");
#endif
        }
    
        bool try_lock()
        {
            int lockState = UNLOCK;
            return std::atomic_compare_exchange_strong(&m_word, &lockState, static_cast<int>(LOCK_SINGLE));
        }
        
        template<typename _Rep, typename _Period>
        bool try_lock_for(const std::chrono::duration<_Rep, _Period>& rtime)
        {
            throw Exception(__CORE_SOURCE, "operation is not supported");
        }
    
        template<typename _Duration>
        bool try_lock_until(const std::chrono::time_point<__clock_t, _Duration>& atime)
        {
            throw Exception(__CORE_SOURCE, "operation is not supported");
        }
    
    private:
       std::atomic<int> m_word;
    };
}

