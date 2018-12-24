#pragma once

#include <string>

namespace core
{
    class SharedObject
    {
    public:
        enum class AccessMod
        {
            READ_WRITE,
            READ
        };
        SharedObject(const std::string& name, AccessMod mod);
        ~SharedObject();
        void Unlink();
        
    private:
        static std::string AddLeadingSlash(const std::string& name);
        
    private:
        int m_handle;
        bool m_linked;
        std::string m_name;
    };
}