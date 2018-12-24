#include "Directory.h"
#include "Assert.h"
#if defined(__linux)
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace core{
    std::string Directory::GetDirctoryFullPath(const std::string& path)
    {
        size_t position = path.find_last_of('/');
        return std::string(path.data(), position != std::string::npos ? position : path.length());
    }
    
    std::string Directory::GetFileName(const std::string &path)
    {
        size_t position = path.find_last_of('/');
        return std::string(path.begin() + position, path.end());
    }
    
    void Directory::CreateDirectory(const std::string &path, std::tuple<Permission, Permission, Permission> &permission)
    {
        #if defined(__linux)
        mode_t mode = 0;
        mode = mode | (char)std::get<PermissionGroup::OTHERS>(permission);
        mode = mode | (((char)std::get<PermissionGroup::GROUP>(permission)) << (PermissionGroup::GROUP * 8));
        mode = mode | (((char)std::get<PermissionGroup::USERS>(permission)) << (PermissionGroup::USERS * 8));
        PLATFORM_VERIFY(0 == ::mkdir(path.c_str(), mode));
        #endif
    }
    
    void Directory::RemoveDirectory(const std::string &path)
    {
        #if defined(__linux)
        PLATFORM_VERIFY(0 == ::rmdir(path.c_str()));
        #endif
    }
}
