#pragma once

#include <atomic>

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
    
    public:
        Mutex();
        void lock();
        bool try_lock();
        void unlock();
    
    private:
       std::atomic<int> m_word;
    };
}

