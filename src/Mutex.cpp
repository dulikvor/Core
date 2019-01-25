#include "Mutex.h"
#if defined(__linux)
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#endif
#include "Exception.h"

namespace core{
    
    Mutex::Mutex(): m_word(UNLOCK) {}
    
    void Mutex::lock()
    {
        int lockState = UNLOCK;
        if(std::atomic_compare_exchange_strong(&m_word, &lockState, static_cast<int>(LOCK_SINGLE)) == false)
        {
            if(lockState != LOCK_MANY)
                lockState = std::atomic_exchange(&m_word, static_cast<int>(LOCK_MANY));
            while(lockState != UNLOCK)
            {

#if defined(__linux)
                syscall(SYS_futex, &m_word, FUTEX_WAIT, LOCK_MANY, nullptr, nullptr, 0);
#else
                throw Exception(__CORE_SOURCE, "wait is not being supported by current platform");
#endif
                lockState = std::atomic_exchange(&m_word, static_cast<int>(LOCK_MANY));
            }
        }
    }
    
    void Mutex::unlock()
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
    
    bool Mutex::try_lock()
    {
        int lockState = UNLOCK;
        return std::atomic_compare_exchange_strong(&m_word, &lockState, static_cast<int>(LOCK_SINGLE));
    }
}
