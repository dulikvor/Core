#pragma once

#include <string>
#include <functional>
#include <type_traits>
#include "Exception.h"

#define ENUMERATION(name) \
public: \
    name(Enumeration e):m_currentEnum(e){} \
    operator Enumeration() const {return m_currentEnum;} \
    std::string ToString() const;\
    static Enumeration FromString(const std::string& str);\
\
public: \
    struct Hash{ \
    Hash() = default; \
    size_t operator()(const name& e) const{ \
        return std::hash<int>{}((int)(Enumeration)e); \
    };\
    size_t operator()(const name* e) const{ \
            return std::hash<int>{}((int)(Enumeration)*e); \
    }; \
    }; \
\
private: \
    struct EnumStringPair\
    {\
        Enumeration enumValue;\
        std::string enumStrName;\
    };\
\
private: \
    Enumeration m_currentEnum; \
    static EnumStringPair m_enumToString[]; \
    const static int numOfEnumValues;\

#define ENUMERATION_NAMING_BEGIN(name)\
    name::EnumStringPair name::m_enumToString[]={

#define ENUMERATION_NAMING_END(name)\
    };\
    const int name::numOfEnumValues = std::extent<decltype(name::m_enumToString)>::value;\
    \
    std::string name::ToString() const\
    {\
        for(int index = 0; index < numOfEnumValues; index++)\
        {\
            if(m_enumToString[index].enumValue == m_currentEnum)\
                return m_enumToString[index].enumStrName;\
        }\
        throw core::Exception(__CORE_SOURCE,"Not all enum values are covered");\
    }\
    \
    name::Enumeration name::FromString(const std::string& str){\
        for(int index = 0; index < numOfEnumValues; index++){\
            if(m_enumToString[index].enumStrName == str) \
                return m_enumToString[index].enumValue; \
        }\
        throw core::Exception(__CORE_SOURCE, "requested string value is not supported - %s", str.c_str());\
    }






