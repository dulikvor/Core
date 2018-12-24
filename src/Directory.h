#pragma once

#include <string>
#include <tuple>

namespace core
{
    class Directory
    {
    public:
        enum Permission
        {
            READ_ONLY = 4,
            WRITE_ONLY = 2,
            EXECUTE_ONLY = 1,
            READ_WRITE_ONLY = 6,
            READ_EXECUTE_ONLY = 5,
            WRITE_EXECUTE_ONLY = 3,
            ALL = 7
        };
        enum PermissionGroup
        {
            OTHERS = 0,
            GROUP = 1,
            USERS = 2
        };
        static std::string GetDirctoryFullPath(const std::string& path);
        static std::string GetFileName(const std::string& path);
        static void CreateDirectory(const std::string& path, std::tuple<Permission, Permission, Permission>& permission);
        static void RemoveDirectory(const std::string& path);
    };
}
