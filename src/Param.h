#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <type_traits>
#include "Exception.h"

namespace core
{
    #define ARGUMENTS short, int, long, float, double, bool, void*, std::string, std::vector<std::string>

    template<typename X, typename... Args>
    struct TypeId{};

    template<typename X>
    struct TypeId<X>
    {
        const static int value = -1;
    };


    template<typename X, typename... Args>
    struct TypeId<X, X, Args...>
    {
        const static int value = 0;
    };

    template<typename X, typename T, typename... Args>
    struct TypeId<X, T, Args...>
    {
        const static int value = TypeId<X, Args...>::value != -1 ? TypeId<X, Args...>::value + 1 : -1;
    };

    class IParam
    {
    public:
	    virtual ~IParam(){}
    };

    template<typename X>
    class Param : public IParam
    {
    public:
        typedef typename std::remove_reference<X>::type Type;

        Param(X&& value, typename std::enable_if<std::is_copy_constructible<
                typename std::remove_reference<X>::type>::value, bool>::type = true) :
                m_typeId(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::forward<X>(value));
        }

        Param(X& value, typename std::enable_if<std::is_copy_constructible<
                typename std::remove_reference<X>::type>::value, bool>::type = true) :
                m_typeId(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::forward<X>(value));
        }

        Param(const X&& value, typename std::enable_if<std::is_copy_constructible<
                typename std::remove_reference<X>::type>::value, bool>::type = true) :
                m_typeId(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::forward<const X>(value));
        }

        Param(const X& value, typename std::enable_if<std::is_copy_constructible<
                typename std::remove_reference<X>::type>::value, bool>::type = true) :
                m_typeId(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::forward<const X>(value));
        }

        virtual ~Param()
        {
            Type* typedBuffer = reinterpret_cast<Type*>(m_rawBuffer);
            typedBuffer->~Type();
            delete[] m_rawBuffer;
        }

        Param(const Param&) = delete;
        void operator = (const Param&) = delete;

        Param(Param&& obj):m_rawBuffer(nullptr)
        {
            m_typeId = obj.m_typeId;
            std::swap(m_rawBuffer, obj.m_rawBuffer);
        }

        template<typename Y, typename std::enable_if<std::__and_<std::is_object<Y>,
                std::is_copy_constructible<Y>>::value, int>::type = 0>
        Y Get() const
        {
            int typeId = TypeId<Y, ARGUMENTS>::value;
            assert(typeId == m_typeId);
            return *reinterpret_cast<const Y*>(m_rawBuffer);
        }

    private:
        int m_typeId;
        char* m_rawBuffer;
    };

    template<typename X>
    inline std::unique_ptr<IParam> MakeParam(X&& val)
    {
        return std::unique_ptr<IParam>(new Param<typename std::remove_cv<
		        typename std::remove_reference<X>::type>::type>(std::forward<X>(val)));
    };
}



