#pragma once

#include <mutex>
#include <condition_variable>
#include "Assert.h"
#include "AsyncTask.h"
namespace  core{

    template<typename T> class Promise;

    template<typename T>
    class Future: public AsyncTask{
    public:
        explicit Future(const std::function<void(void)>& requestedPoint):AsyncTask(requestedPoint){}
        virtual ~Future(){}
        T Get() const {
            {
                std::unique_lock <std::mutex> localLock(m_promiseMut);
                m_promiseCv.wait(localLock, [this] { return m_valueSet; });
            }
            return m_value;
        }
    private:
        friend class Promise<T>;
        void SetValue(const T& value){
            std::unique_lock<std::mutex> lock(m_promiseMut);
            VERIFY(m_valueSet == false, "Value can only be set once.");
            m_value = value;
            m_valueSet = true;
            m_promiseCv.notify_one();
        }

    private:
        mutable std::condition_variable m_promiseCv;
        mutable std::mutex m_promiseMut;
        bool m_valueSet;
        T m_value;
    };
}
