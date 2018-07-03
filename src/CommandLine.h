#pragma once

#include <list>
#include <utility>
#include <string>
#include "Export.h"

namespace core
{
    class CommandLine
    {
    public:
        CORE_EXPORT ~CommandLine(){}
        CORE_EXPORT static CommandLine& Instance();
        CORE_EXPORT void Parse(int argc, char const** argv);
        CORE_EXPORT char const * const GetArgument(char const * const argumentName) const;

    private:
        CommandLine(){}
        bool ParseArgument(const char* argument, std::string& argName, std::string& argValue);

    private:
        using ArgumentList = std::list<std::pair<std::string, std::string>>;
         ArgumentList m_arguments;

    };
}
