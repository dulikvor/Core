#pragma once

#include <functional>
#include <type_traits>
#include <memory>

namespace core {
    template<typename T>
    struct defualtDelter{
        constexpr defualtDelter() = default;
        void operator()(T *obj) {
            delete obj;
        }
    };

    template<typename T>
    class Deleter {
    public:
        Deleter(T *obj, std::function<void(T*)> deleterFunctor = defualtDelter<T>())
                : m_obj(obj), m_deleterFunctor(deleterFunctor){}
        ~Deleter() { m_deleterFunctor(m_obj); }

    private:
        T *m_obj;
        std::function<void(T*)> m_deleterFunctor;
    };
}