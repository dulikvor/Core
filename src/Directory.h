#pragma once

#include <string>

namespace core
{
    class Directory
    {
    public:
        static std::string GetDir(const std::string& path);
        static std::string GetProcessName(const std::string& path);
    };
}
