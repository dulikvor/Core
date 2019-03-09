#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace core
{
    template <typename T>
    class SyncQueue
    {
    public:
        typedef T value_type;
        
        SyncQueue() = default;
        ~SyncQueue() = default;
        //Push receives a list of elements of type T, copying each element into the queue and notifying
        //upon their insertion.
        void push(const std::vector<T>& elements)
        {
            std::unique_lock<std::mutex>(m_mutex);
            for(const T& element : elements)
            {
                m_queue.push(element);
                m_conditionVar.notify_one();
            }
        }

        void push(const T& element)
        {
            std::unique_lock<std::mutex> localLock(m_mutex);
            m_queue.push(element);
            m_conditionVar.notify_one();
        }
        //TryAndPop verifies if the stored queue is empty or not - if empty it will not assign a stored value into the received element.
        // and will return false (no element was retrieved), if not empty it will fetch the top element, assigning it into the received
        // element and returning true as a response (an element was fetched).
        bool try_pop(T& element)
        {
            std::unique_lock<std::mutex> localLock(m_mutex);
            if(m_queue.empty())
                return false;
            
            element = m_queue.front();
            m_queue.pop();
            return true;
        }

        T pop()
        {
            std::unique_lock<std::mutex> localLock(m_mutex);
            m_conditionVar.wait(localLock, [this]()->bool{return !m_queue.empty();});
            T ret = m_queue.front();
            m_queue.pop();
            return ret;
        }

        bool is_empty() const
        {
            std::unique_lock<std::mutex> localLock(m_mutex);
            return m_queue.empty();
        }

    private:
        std::queue<T> m_queue;
        mutable std::condition_variable m_conditionVar;
        mutable std::mutex m_mutex;
    };
}
