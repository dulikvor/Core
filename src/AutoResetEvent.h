#pragma once

#include <mutex>
#include <condition_variable>

namespace core
{
    class AutoResetEvent
    {
    public:
        AutoResetEvent():m_set(false){}
        void Set();
        void WaitOne();
    private:
        std::condition_variable m_conditionalVariable;
        std::mutex m_mutex;
        bool m_set;
    };
}


