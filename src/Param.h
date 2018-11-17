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
    #define ARGUMENTS short, int, long, float, double, bool, void*, char*, std::string, std::vector<std::string>

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

    #define IS_INTEGRAL(NAME, TYPE) \
    bool Is##NAME() const\
    {   \
        return m_typeId == TypeId<TYPE, ARGUMENTS>::value; \
    }

    class Param
    {
    public:
        Param(int typeId):m_typeId(typeId){}
	    virtual ~Param(){}

        IS_INTEGRAL(Short,       short);
        IS_INTEGRAL(Int,         int);
        IS_INTEGRAL(Long,        long);
        IS_INTEGRAL(Float,       float);
        IS_INTEGRAL(Double,      double);
        IS_INTEGRAL(Bool,        bool);
        IS_INTEGRAL(Pointer,     void*);
        IS_INTEGRAL(CtypeS,      const char*);
        IS_INTEGRAL(String,      std::string);
        IS_INTEGRAL(StringArray, std::vector<std::string>);

    protected:
        int m_typeId;
    };

    template<typename X>
    class TypedParam : public Param
    {
    public:
        typedef typename std::remove_reference<X>::type Type;

        TypedParam(X&& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::move(value));
        }

        template<typename T = X, typename = typename std::enable_if<!std::is_same<T, char*>::value, bool>::type>
        TypedParam(X& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(value);
        }

        TypedParam(const X&& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(std::move(value));
        }

        template<typename T = X, typename = typename std::enable_if<!std::is_same<T, char*>::value, bool>::type>
        TypedParam(const X& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(value);
        }

        TypedParam(const char*& value) :
                Param(TypeId<char*, ARGUMENTS>::value)
        {
            size_t size = strlen(value);
            m_rawBuffer = new char[size + 1];
            memcpy(m_rawBuffer, value, size);
            m_rawBuffer[size] = '\0';
        }

        virtual ~TypedParam()
        {
            Type* typedBuffer = reinterpret_cast<Type*>(m_rawBuffer);
            typedBuffer->~Type();
            delete[] m_rawBuffer;
        }

        template<typename T>
        TypedParam(const TypedParam<T>& object) :
                Param(object.m_typeId)
        {
            static_assert(std::is_same<Type, typename TypedParam<T>::Type>::value == true, "Type mismatch");
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(*reinterpret_cast<Type*>(object.m_rawBuffer));
        }

        TypedParam(const TypedParam<char*>& object) :
                Param(object.m_typeId)
        {
            static_assert(std::is_same<Type, typename TypedParam<char*>::Type>::value == true, "Type mismatch");
            size_t size = strlen(object.m_rawBuffer);
            m_rawBuffer = new char[size + 1];
            memcpy(m_rawBuffer, object.m_rawBuffer, size);
            m_rawBuffer[size] = '\0';
        }

        template<typename T>
        TypedParam& operator=(const TypedParam<T>& object)
        {
            static_assert(std::is_same<Type, typename TypedParam<T>::Type>::value == true, "Type mismatch");
            m_typeId = object.m_typeId;
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(*reinterpret_cast<Type*>(object.m_rawBuffer));
            return *this;
        }

        TypedParam& operator=(const TypedParam<char*>& object)
        {
            static_assert(std::is_same<Type, typename TypedParam<char*>::Type>::value == true, "Type mismatch");
            m_typeId = object.m_typeId;
            size_t size = strlen(object.m_rawBuffer);
            m_rawBuffer = new char[size];
            memcpy(m_rawBuffer, object.m_rawBuffer, size);
            return *this;
        }

        TypedParam(TypedParam&& obj):m_rawBuffer(nullptr)
        {
            m_typeId = obj.m_typeId;
            std::swap(m_rawBuffer, obj.m_rawBuffer);
        }

        template<typename Y, typename = typename std::enable_if<!std::is_same<Y, char*>::value, bool>::type>
        const Y& Get() const
        {
            assert((TypeId<Y, ARGUMENTS>::value) == m_typeId);
            return *reinterpret_cast<const Y*>(m_rawBuffer);
        }

        template<typename Y, typename = typename std::enable_if<std::is_same<Y, char*>::value, bool>::type>
        char* Get() const
        {
            assert((TypeId<char*, ARGUMENTS>::value) == m_typeId);
            return m_rawBuffer;
        }

    private:
        char* m_rawBuffer;
    };

    template<typename X, typename std::enable_if<!std::is_array<typename std::remove_reference<X>::type>::value, bool>::type = true>
    inline std::unique_ptr<Param> MakeParam(X&& val)
    {
        return std::unique_ptr<Param>(new TypedParam<typename std::remove_cv<
		        typename std::remove_reference<X>::type>::type>(std::forward<X>(val)));
    }

    template<typename X, typename std::enable_if<std::is_array<typename std::remove_reference<X>::type>::value &&
            std::is_lvalue_reference<X>::value, bool>::type = true>
    inline std::unique_ptr<Param> MakeParam(X&& val)
    {
        const char* _val = val; //Force a conversion to T = const char*&
        return std::unique_ptr<Param>(new TypedParam<char*>(_val));
    }
}



