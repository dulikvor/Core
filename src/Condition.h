#pragma once

#include <atomic>
#include "Mutex.h"

namespace core{
    
    class Condition
    {
    private:
        enum Command : int
        {
            SLEEP = 0,
            NOTIFY_ONE,
            NOTIFY_ALL
        };
        
    public:
        Condition();
        void Signal(Command command);
        void Wait(Mutex& mutex);
        
    private:
        volatile std::atomic<int> m_command;
        std::atomic<int> m_waitersCount;
        Mutex m_wakeLock;
    };
}
