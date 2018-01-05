#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "SyncQueue.h"
#include "Thread.h"

namespace core
{
    class AsyncTask;
}

namespace core
{
    class AsyncExecutor
    {
    public:
        explicit AsyncExecutor(int threadPoolSize = 1, bool activeAll = true);
        ~AsyncExecutor();

        virtual void SpawnTask(AsyncTask* task){m_taskQueue.Push(task);}
        void IncActiveThreads();
        void DecActiveThreads();


    private:
        void EntryPoint(int id);

    private:
        std::vector<std::unique_ptr<Thread>> m_threadPool;
        SyncQueue<AsyncTask*> m_taskQueue;
        std::atomic_bool m_running;
        std::vector<std::mutex*> m_threadsLocks;
        std::vector<std::condition_variable*> m_threadsCondVar;
        std::vector<bool> m_activeThreads;
        int m_lastActiveThread;
        int m_threadsPoolSize;
        std::mutex m_incDecLock;
    };
}
