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
    public:
        GeneralParams(){}
        ~GeneralParams(){}

        template<typename X>
        void AddParam(const char* key, X&& value)
        {
            typedef std::pair<std::string, std::unique_ptr<Param>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;};
            if(std::find_if(m_values.begin(), m_values.end(), comparator) == m_values.end())
            {
                auto param = MakeParam(std::forward<X>(value));
                m_values.push_back(std::make_pair(key, std::move(param)));
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
            typedef std::pair<std::string, std::unique_ptr<Param>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_values.begin(), m_values.end(), comparator);
            if(it == m_values.end())
                throw Exception(__CORE_SOURCE, "Non existing parameter was requested %s", key);
            else
            {
                TypedParam<T>& param = static_cast<TypedParam<T>&>(*it->second);
                return param.template Get<T>();
            }
        }

        bool Exists( const char* key ) const
        {
            typedef std::pair<std::string, std::unique_ptr<Param>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return pair.first == key;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_values.begin(), m_values.end(), comparator);
            return it != m_values.end();
        }

    private:
        std::vector<std::pair<std::string, std::unique_ptr<Param>>> m_values;
    };
}
