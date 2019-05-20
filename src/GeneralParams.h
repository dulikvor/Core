#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <string>
#include <memory>
#include <type_traits>
#include "Assert.h"
#include "Exception.h"
#include "Param.h"

namespace core
{
    class GeneralParams
    {
    private:
        typedef std::unique_ptr<Param> ParamPtr;
        typedef std::pair<std::string, ParamPtr> ParamPair;
        typedef std::vector<ParamPair> Params;
        
    public:
        GeneralParams() = default;
        ~GeneralParams() = default;
        GeneralParams(const GeneralParams& obj)
        {
            Copy(obj);
        }
        GeneralParams& operator=(const GeneralParams& rhs)
        {
            Copy(rhs);
        }
        GeneralParams(GeneralParams&& obj)
            :m_params(std::move(obj.m_params)){}
        GeneralParams& operator=(GeneralParams&& rhs)
        {
            m_params = std::move(rhs.m_params);
        }
        template<typename X>
        void AddParam(const char* key, X&& value)
        {
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;};
            if(std::find_if(m_params.begin(), m_params.end(), comparator) == m_params.end())
            {
                auto param = MakeParam(std::forward<X>(value));
                m_params.push_back(std::make_pair(key, std::move(param)));
            }
            else
            {
                throw core::Exception(__CORE_SOURCE, "Requested key already exists - %s", key);
            }
        }
        template<typename T, typename std::enable_if<std::is_object<T>::value &&
                std::is_copy_constructible<T>::value, int>::type = 0>
        T Get(const char* key) const
        {
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_params.begin(), m_params.end(), comparator);
            if(it == m_params.end())
                throw Exception(__CORE_SOURCE, "Non existing parameter was requested %s", key);
            else
            {
                TypedParam<T>& param = static_cast<TypedParam<T>&>(*it->second);
                return param.template Get<T>();
            }
        }
        bool Exists( const char* key ) const
        {
        
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_params.begin(), m_params.end(), comparator);
            return it != m_params.end();
        }
        
    private:
        void Copy(const GeneralParams& obj) {
            Params params;
            for (auto &paramPair: obj.m_params)
            {
                static ParamPtr toParam;
                Param& param = *paramPair.second;
    
                if (param.IsShort()) {
                    TypedParam<short> &typedElement = static_cast<TypedParam<short> &>(param);
                    toParam.reset(new TypedParam<short>(typedElement));
                } else if (param.IsInt()) {
                    TypedParam<int> &typedElement = static_cast<TypedParam<int> &>(param);
                    toParam.reset(new TypedParam<int>(typedElement));
                } else if (param.IsLong()) {
                        TypedParam<long> &typedElement = static_cast<TypedParam<long> &>(param);
                        toParam.reset(new TypedParam<long>(typedElement));
                } else if (param.IsBool()) {
                    TypedParam<bool> &typedElement = static_cast<TypedParam<bool> &>(param);
                    toParam.reset(new TypedParam<bool>(typedElement));
                } else if (param.IsFloat()) {
                    TypedParam<float> &typedElement = static_cast<TypedParam<float> &>(param);
                    toParam.reset(new TypedParam<float>(typedElement));
                } else if (param.IsDouble()) {
                    TypedParam<double> &typedElement = static_cast<TypedParam<double> &>(param);
                    toParam.reset(new TypedParam<double>(typedElement));
                } else if (param.IsPointer()) {
                    TypedParam<void*> &typedElement = static_cast<TypedParam<void*> &>(param);
                    toParam.reset(new TypedParam<void*>(typedElement));
                } else if (param.IsCtypeS()) {
                    TypedParam<char*> &typedElement = static_cast<TypedParam<char*> &>(param);
                    toParam.reset(new TypedParam<char*>(typedElement));
                } else if (param.IsString()) {
                    TypedParam<std::string> &typedElement = static_cast<TypedParam<std::string> &>(param);
                    toParam.reset(new TypedParam<std::string>(typedElement));
                } else if (param.IsStringArray()) {
                    TypedParam<std::vector<std::string>> &typedElement = static_cast<TypedParam<std::vector<std::string>> &>(param);
                    toParam.reset(new TypedParam<std::vector<std::string>>(typedElement));
                }
                
                params.emplace_back(std::make_pair(paramPair.first, std::move(toParam)));
            }
            std::swap(m_params, params);
        }

    private:
        Params m_params;
    };
}
