#include "TimedAsyncExecutor.h"

using namespace std;
using namespace std::chrono;

namespace core
{
    TimedAsyncExecutor::TimedAsyncExecutor(int threadPoolSize): AsyncExecutor(threadPoolSize)
    {
        m_running = true;
        m_publishingThread.reset(new Thread("TimedTasks_Pub", 
                    bind(&TimedAsyncExecutor::TasksPublisher, this)));
        m_publishingThread->Start();
    }

    TimedAsyncExecutor::~TimedAsyncExecutor()
    {
        m_running = false;
        m_cv.notify_one();
        m_publishingThread->Join();
    }

    void TimedAsyncExecutor::SpawnTask(AsyncTask* task)
    {
        SpawnTimedTask(task, system_clock::now());
    }

    void TimedAsyncExecutor::SpawnTimedTask(AsyncTask* task, system_clock::time_point dueTime) {
        unique_lock<mutex> lock(m_mut);
        //Verify the next min time to wait for
        auto currentMinTime =
                m_upcomingTasks.size() == 0 ? system_clock::time_point::max() : m_upcomingTasks.begin()->first;
        m_upcomingTasks.insert(make_pair(dueTime, task));
        if (dueTime < currentMinTime) {
            m_cv.notify_one();
        }
    }

    void TimedAsyncExecutor::TasksPublisher()
    {
        while(m_running)
        {
            unique_lock<mutex> lock(m_mut);
            auto currentMinTime = m_upcomingTasks.size() == 0 ? system_clock::time_point::max() : m_upcomingTasks.begin()->first;
            m_cv.wait_until(lock, currentMinTime);
            if(m_upcomingTasks.size() > 0)
            {
                AsyncExecutor::SpawnTask(m_upcomingTasks.begin()->second);
                m_upcomingTasks.erase(m_upcomingTasks.begin());
            }    
        }
    }
}


