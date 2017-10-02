#pragma once

#include <memory>
#include "Future.h"

namespace core{

    template<typename T> class Future;

    template<typename T>
    class Promise
    {
    public:

        void SetValue(const T& value){
            m_future->SetValue(value);
        }

        std::shared_ptr<Future<T>> GetFuture() {
            m_future = std::make_shared<Future<T>>();
            return m_future;
        }

    private:
        std::shared_ptr<Future<T>> m_future;
    };
}
