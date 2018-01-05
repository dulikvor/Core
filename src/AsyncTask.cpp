#include "AsyncTask.h"
#include "Exception.h"
#include "Promise.h"

using namespace std;

namespace core
{
    AsyncTask::AsyncTask(const std::function<void(void)> &requestedPoint): m_requestedEntryPoint(requestedPoint),
                 m_state(AsyncTaskState::CREATED){}

    AsyncTask::AsyncTask(const AsyncTask& obj){
        m_requestedEntryPoint = obj.GetRequestedEntryPoint();
        m_state = obj.GetState();
        m_failureReason = obj.GetFailureReason();
    }

    AsyncTask& AsyncTask::operator=(const AsyncTask& obj){
        m_requestedEntryPoint = obj.GetRequestedEntryPoint();
        m_state = obj.GetState();
        m_failureReason = obj.GetFailureReason();
        return *this;
    }

    void AsyncTask::Start()
    {
        m_state = AsyncTaskState::RUNNING;
        m_requestedEntryPoint();
        m_state = AsyncTaskState::COMPLETED;
        {
            unique_lock<std::mutex> localLock(m_waitMut);
            m_waitCv.notify_one();
        }
    }


    void AsyncTask::Wait()
    {
        unique_lock<std::mutex> localLock(m_waitMut);
        m_waitCv.wait(localLock, [&]{return m_state == AsyncTaskState::CANCELED ||
                    m_state == AsyncTaskState::COMPLETED;});
        if(m_state == AsyncTaskState::CANCELED)
            throw Exception(SOURCE, "%s", m_failureReason.c_str());
    }

    void AsyncTask::NotifyOnFailure()
    {
        unique_lock<std::mutex> localLock(m_waitMut);
        m_state = AsyncTaskState::CANCELED;
        m_waitCv.notify_one();
    }
}

