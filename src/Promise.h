#pragma once

#include <memory>
#include "Future.h"

namespace core{

    class AsyncTask;

    template<typename T>
    class Promise
    {
    public:

        void SetValue(const T& value){
            static_cast<Future<T>*>(m_task.get())->SetValue(value);
        }

        std::shared_ptr<AsyncTask> GetTask(const std::function<void(void)>& requestedPoint) {
            m_task = std::shared_ptr<AsyncTask>(new Future<T>(requestedPoint));
            return m_task;
        }

    private:
        std::shared_ptr<AsyncTask> m_task;
    };
}
