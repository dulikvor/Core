#pragma once

#include <atomic>
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
        
        Condition();
        void Signal(Command command);
        void Wait(Mutex& mutex);
    
    private:
        volatile std::atomic<int> m_command;
        std::atomic<int> m_waitersCount;
        Mutex m_wakeLock;
    };
    
    class ConditionVariable
    {
    public:
        ConditionVariable();
        ConditionVariable(const ConditionVariable&) = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;
        
        void wait(Mutex& mutex);
        template<typename Predict>
        void wait(const Predict& predict, Mutex& mutex)
        {
            while(predict() == false)
                m_condition.Wait(mutex);
        }
        
        void notify_one();
        void notify_all();
        
    private:
        Condition m_condition;
    };
    
}
