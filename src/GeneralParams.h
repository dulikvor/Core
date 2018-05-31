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
            typedef std::pair<const char*, std::unique_ptr<IParam>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return strcmp(pair.first, key) == 0;};
            if(std::find_if(m_values.begin(), m_values.end(), comparator) == m_values.end())
            {
                auto param = MakeParam(std::forward<X>(value));
                m_values.push_back(std::make_pair(key, std::move(param)));
            }
            else
            {
                throw core::Exception(__CORE_SOURCE, "A requested key was already existed - %s", key);
            }
        }

        template<typename T, typename std::enable_if<std::__and_<std::is_object<T>,
                std::is_copy_constructible<T>>::value, int>::type = 0>
        T Get(const char* key) const
        {
            typedef std::pair<const char*, std::unique_ptr<IParam>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return strcmp(pair.first, key) == 0;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_values.begin(), m_values.end(), comparator);
            if(it == m_values.end())
                throw Exception(__CORE_SOURCE, "Non existing parameter was requested %s", key);
            else
            {
                Param<T>& param = static_cast<Param<T>&>(*it->second);
                return param.Get<T>();
            }
        }

        bool Exists( const char* key ) const
        {
            typedef std::pair<const char*, std::unique_ptr<IParam>> ParamPair;
            auto comparator = [&key](const ParamPair& pair) -> bool {return strcmp(pair.first, key) == 0;}; //redundancy from above, but never mind :)
            std::vector<ParamPair>::const_iterator it = std::find_if(m_values.begin(), m_values.end(), comparator);
            return it != m_values.end();
        }

    private:
        std::vector<std::pair<const char*, std::unique_ptr<IParam>>> m_values;
    };
}
