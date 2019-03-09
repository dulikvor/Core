#pragma once

#include <atomic>
#include <mutex>
#include <limits>
#if defined(__linux)
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#endif
#include "Mutex.h"

namespace core{
    
    class Condition
    {
    public:
        enum Command : int
        {
            SLEEP = 0,
            NOTIFY_ONE,
            NOTIFY_ALL
        };
    
        Condition(): m_command(SLEEP), m_waitersCount(0){}
        Condition(const Condition&) = delete;
        Condition& operator=(const Condition&) = delete;
        
        void signal(Command command)
        {
            m_wakeLock.lock();
            if(m_waitersCount == 0)
            {
                m_wakeLock.unlock();
                return;
            }
            m_command.store(command, std::memory_order_relaxed);
#if defined(__linux)
            syscall(SYS_futex, &m_command, FUTEX_WAKE, command == NOTIFY_ONE ? 1 : std::numeric_limits<int>::max(), nullptr, nullptr, 0);
#else
            throw Exception(__CORE_SOURCE, "wake is not being supported by current platform");
#endif
    
            while(m_command.load(std::memory_order_relaxed) != SLEEP && m_waitersCount.load(std::memory_order_relaxed) != 0){}
    
            m_wakeLock.unlock();
        }
        
        void wait(std::unique_lock<Mutex>& mutex)
        {
            m_waitersCount++;
            mutex.unlock();
            while (true) {
                while (m_command.load(std::memory_order_acquire) == SLEEP)
#if defined(__linux)
                    syscall(SYS_futex, &m_command, FUTEX_WAIT, SLEEP, nullptr, nullptr, 0);
#else
                throw Exception(__CORE_SOURCE, "wait is not being supported by current platform");
#endif
        
                int fromCommand_one = NOTIFY_ONE;
                if (m_command.compare_exchange_strong(fromCommand_one, static_cast<int>(SLEEP),
                                                      std::memory_order_relaxed)) {
                    m_waitersCount--;
                    mutex.lock();
                    return;
                } else if (m_command.load(std::memory_order_relaxed) == NOTIFY_ALL) {
                    if (m_waitersCount.fetch_sub(1, std::memory_order_relaxed) == 1)
                        m_command.store(SLEEP, std::memory_order_relaxed);
            
                    mutex.lock();
                    return;
                }
            }
        }
    
    private:
        volatile std::atomic<int> m_command;
        std::atomic<int> m_waitersCount;
        Mutex m_wakeLock;
    };
    
    class ConditionVariable
    {
    public:
        ConditionVariable()=default;
        ConditionVariable(const ConditionVariable&) = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;
        
        template<typename Predict>
        void wait(std::unique_lock<Mutex>& lock, const Predict& predict)
        {
            while(predict() == false)
                m_condition.wait(lock);
        }
    
        void wait(std::unique_lock<core::Mutex> &mutex){ m_condition.wait(mutex); }
        void notify_one(){ m_condition.signal(Condition::NOTIFY_ONE); }
        void notify_all(){ m_condition.signal(Condition::NOTIFY_ALL); }
        
    private:
        Condition m_condition;
    };
    
}
