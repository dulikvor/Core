#include "AsyncExecutor.h"
#include <string>
#include <functional>
#include "Exception.h"
#include "AsyncTask.h"
#include "Assert.h"

using namespace std;

namespace core
{
    AsyncExecutor::AsyncExecutor(int threadPoolSize, bool activeAll)
        :m_threadsPoolSize(threadPoolSize)
    {
        ASSERT(threadPoolSize > 0);
        activeAll ? m_lastActiveThread = threadPoolSize - 1 : m_lastActiveThread = -1;
        m_activeThreads.resize(threadPoolSize, activeAll);
        m_threadsLocks.resize(threadPoolSize, new mutex());
        m_threadsCondVar.resize(threadPoolSize, new condition_variable());
        for(int index = 0; index < threadPoolSize; index++)
        {
            m_threadPool.emplace_back(new Thread(string("AsyncExec_") + 
                        to_string(index), bind(&AsyncExecutor::EntryPoint, this, index)));
            m_threadPool.back()->Start();
        }
    }

    void AsyncExecutor::IncActiveThreads()
    {
        unique_lock<mutex>(m_incDecLock);
        {
            if(m_lastActiveThread == m_threadsPoolSize - 1)
                return;
            {
                unique_lock<mutex> lock(*m_threadsLocks[++m_lastActiveThread]);
                m_activeThreads[m_lastActiveThread] = true;
                m_threadsCondVar[m_lastActiveThread]->notify_one();
            }
        }
    }

    void AsyncExecutor::DecActiveThreads()
    {
        unique_lock<mutex>(m_incDecLock);
        {
            if(m_lastActiveThread == -1)
                return;
            {
                unique_lock<mutex> lock(*m_threadsLocks[m_lastActiveThread]);
                m_activeThreads[m_lastActiveThread--] = false;
            }
        }
    }

    AsyncExecutor::~AsyncExecutor()
    {
        Logger::Instance().PrintStack();
        for(int index = 0; index < (int)m_threadPool.size(); index++)
        {
            m_taskQueue.Push(nullptr);
        }
        for(const unique_ptr<Thread>& thread : m_threadPool)
        {
            thread->Join();
        }
        for(condition_variable* condVar : m_threadsCondVar)
            delete condVar;
        for(mutex* mut : m_threadsLocks)
            delete mut;
    }

    void AsyncExecutor::EntryPoint(int id)
    {
        while(1)
        {
            {
                unique_lock<mutex> localLock(*m_threadsLocks[id]);
                m_threadsCondVar[id]->wait(localLock, [this, &id]()->bool{
                        return m_activeThreads[id] == true;});
            }
            AsyncTask* task = m_taskQueue.Pop();
            if(task != nullptr)
            {
                try
                {
                    task->Start();
                }
                catch(const Exception& e)
                {
                    task->SetFailureReason(e.GetMessage());
                    task->NotifyOnFailure();
                }
            }
            else
                break;
        }
    }
}

