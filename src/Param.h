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
    struct typeid_impl{};

    template<typename X>
    struct typeid_impl<X>
    {
        const static int value = -1;
    };


    template<typename X, typename... Args>
    struct typeid_impl<X, X, Args...>
    {
        const static int value = 0;
    };

    template<typename X, typename T, typename... Args>
    struct typeid_impl<X, T, Args...>
    {
        const static int value = typeid_impl<X, Args...>::value != -1 ? typeid_impl<X, Args...>::value + 1 : -1;
    };

    template<typename X, typename... Args>
    struct IsTypeIdExists
    {
        const static bool value = typeid_impl<X, Args...>::value != -1;
    };

    template<typename X, typename... Args>
    struct TypeId
    {
        const static int value = typeid_impl<X, Args...>::value;
    };

#define IS_INTEGRAL(NAME, TYPE) \
    bool Is##NAME() const\
    {   \
        return m_typeId == TypeId<TYPE, ARGUMENTS>::value; \
    }

    class Param
    {
    public:
        Param(int typeId):m_typeId(typeId)
        {
            if(m_typeId == -1)
                throw core::Exception(__CORE_SOURCE, "Non supported type");
        }
        virtual ~Param(){}

        IS_INTEGRAL(Short,       short);
        IS_INTEGRAL(Int,         int);
        IS_INTEGRAL(Long,        long);
        IS_INTEGRAL(Float,       float);
        IS_INTEGRAL(Double,      double);
        IS_INTEGRAL(Bool,        bool);
        IS_INTEGRAL(Pointer,     void*);
        IS_INTEGRAL(CtypeS,      char*);
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

        TypedParam(X&& value) :
                Param(TypeId<Type, ARGUMENTS>::value), m_rawBuffer(nullptr)
        {
            if(m_typeId == TypeId<char*, ARGUMENTS>::value)
            {
                char** valuePtr = const_cast<char**>(reinterpret_cast<char*const*>(&value));
                m_rawBuffer = *valuePtr;
                *valuePtr = nullptr;
            }
            else if(m_typeId == TypeId<void*, ARGUMENTS>::value)
            {
                static_assert(sizeof(void*) == sizeof(char*), "A size mismatch");
                char** valuePtr = const_cast<char**>(reinterpret_cast<char*const*>(&value));
                m_rawBuffer = *valuePtr;
                if(*valuePtr != nullptr)
                    *valuePtr = nullptr;
            }
            else
            {
                m_rawBuffer = new char[sizeof(Type)];
                new (m_rawBuffer) Type(std::move(value));
            }
        }

        TypedParam(X& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            if(m_typeId == TypeId<char*, ARGUMENTS>::value)
            {
                char** valuePtr = reinterpret_cast<char**>(const_cast<X*>(&value));
                size_t size = strlen(*valuePtr);
                m_rawBuffer = new char[size + 1];
                memcpy(m_rawBuffer, *valuePtr, size);
                m_rawBuffer[size] = '\0';
            }
            else if(m_typeId == TypeId<void*, ARGUMENTS>::value)
            {
                static_assert(sizeof(void*) == sizeof(char*), "A size mismatch");
                char*const* valuePtr = reinterpret_cast<char*const*>(&value);
                m_rawBuffer = *valuePtr;
            }
            else
            {
                m_rawBuffer = new char[sizeof(Type)];
                new (m_rawBuffer) Type(value);
            }
        }

        TypedParam(const X& value) : Param(TypeId<Type, ARGUMENTS>::value) //const char* and const void* will not be diverted to here due to the fact const X&, X=char*->char* const&
        {
            m_rawBuffer = new char[sizeof(Type)];
            new (m_rawBuffer) Type(value);
        }

        TypedParam(const char*& value) : Param(TypeId<Type, ARGUMENTS>::value)
        {
            static_assert(std::is_same<char*, Type>::value,"Type mismatch - Type!=char*");
            size_t size = strlen(value);
            m_rawBuffer = new char[size + 1];
            memcpy(m_rawBuffer, value, size);
            m_rawBuffer[size] = '\0';
        }

        TypedParam(const void*& value) : Param(TypeId<Type, ARGUMENTS>::value), m_rawBuffer((char*)const_cast<void*>(value))
        {
            static_assert(std::is_same<void*, Type>::value,"Type mismatch - Type!=void*");
        }

        virtual ~TypedParam()
        {
            if(m_typeId == TypeId<void*, ARGUMENTS>::value) //When Type==void* Param holds no ownership on the data.
                return;
            Type* typedBuffer = reinterpret_cast<Type*>(m_rawBuffer);
            typedBuffer->~Type();
            delete[] m_rawBuffer;
        }

        TypedParam(const TypedParam& obj) :
                Param(obj.m_typeId)
        {
            if(m_typeId == TypeId<char*, ARGUMENTS>::value)
            {
                size_t size = strlen(obj.m_rawBuffer);
                m_rawBuffer = new char[size + 1];
                memcpy(m_rawBuffer, obj.m_rawBuffer, size);
                m_rawBuffer[size] = '\0';
            }
            else if(m_typeId == TypeId<void*, ARGUMENTS>::value)
            {
                static_assert(sizeof(void*) == sizeof(char*), "A size mismatch");
                char*const* valuePtr = reinterpret_cast<char*const*>(&obj.m_rawBuffer);
                m_rawBuffer = *valuePtr;
            }
            else
            {
                m_rawBuffer = new char[sizeof(Type)];
                new (m_rawBuffer) Type(*reinterpret_cast<Type*>(obj.m_rawBuffer));
            }
        }

        TypedParam& operator=(const TypedParam& obj)
        {
            m_typeId = obj.m_typeId;
            if(m_typeId == TypeId<char*, ARGUMENTS>::value)
            {
                size_t size = strlen(obj.m_rawBuffer);
                m_rawBuffer = new char[size + 1];
                memcpy(m_rawBuffer, obj.m_rawBuffer, size);
                m_rawBuffer[size] = '\0';
            }
            else if(m_typeId == TypeId<void*, ARGUMENTS>::value)
            {
                static_assert(sizeof(void*) == sizeof(char*), "A size mismatch");
                char*const* valuePtr = reinterpret_cast<char*const*>(&obj.m_rawBuffer);
                m_rawBuffer = *valuePtr;
            }
            else
            {
                m_rawBuffer = new char[sizeof(Type)];
                new (m_rawBuffer) Type(*reinterpret_cast<Type*>(obj.m_rawBuffer));
            }
            return *this;
        }

        TypedParam(TypedParam&& obj):m_rawBuffer(nullptr)
        {
            m_typeId = obj.m_typeId;
            std::swap(m_rawBuffer, obj.m_rawBuffer);
        }

        TypedParam& operator=(TypedParam&& obj)
        {
            m_typeId = obj.m_typeId;
            m_rawBuffer = nullptr;
            std::swap(m_rawBuffer, obj.m_rawBuffer);
            return *this;
        }

        template<typename Y, typename = typename std::enable_if<!std::is_pointer<Y>::value, bool>::type>
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

        template<typename Y, typename = typename std::enable_if<std::is_same<Y, void*>::value, bool>::type>
        void* Get() const
        {
            assert((TypeId<void*, ARGUMENTS>::value) == m_typeId);
            return (void*)m_rawBuffer;
        }

    private:
        char* m_rawBuffer;
    };

    template<typename X, typename std::enable_if<!std::is_array<typename std::remove_reference<X>::type>::value &&
            !std::is_pointer<typename std::remove_reference<X>::type>::value, bool>::type = true> //All integral types.
    inline std::unique_ptr<Param> MakeParam(X&& val)
    {
        return std::unique_ptr<Param>(new TypedParam<typename std::remove_cv<
                typename std::remove_reference<X>::type>::type>(std::forward<X>(val)));
    }

    template<typename X, typename std::enable_if<!std::is_array<typename std::remove_reference<X>::type>::value &&
         std::is_pointer<typename std::remove_reference<X>::type>::value, bool>::type = true> //For plain old data.
    inline std::unique_ptr<Param> MakeParam(X&& val)
    {
        return std::unique_ptr<Param>(new TypedParam<typename std::add_pointer<typename std::remove_cv<
                typename std::remove_pointer<typename std::remove_reference<X>::type>::type>::type>::type>(std::forward<X>(val)));
    }

    template<typename X, typename std::enable_if<std::is_array<typename std::remove_reference<X>::type>::value &&
                                                 std::is_lvalue_reference<X>::value, bool>::type = true>
    inline std::unique_ptr<Param> MakeParam(X&& val) //For array
    {
        char* _val = const_cast<char*>(val); //Force a conversion to T = const char*&
        return std::unique_ptr<Param>(new TypedParam<char*>(_val));
    }
}



