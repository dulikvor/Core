#include "SharedObject.h"
#if defined(__linux)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm-generic/errno-base.h>
#endif
#include "Exception.h"
#include "Directory.h"
#include "Assert.h"

namespace core{
    SharedObject::SharedObject(const std::string &name, AccessMod mod)
    {
        m_name = AddLeadingSlash(const_cast<std::string&>(name));
        int flag = 0;
        switch(mod)
        {
            case AccessMod::READ:
                flag = O_RDONLY;
                break;
            case AccessMod::READ_WRITE:
                flag = O_RDWR;
                break;
            default:
                throw Exception(__CORE_SOURCE, "Non supported mod");
        }
        mode_t permission = (Directory::READ_WRITE_ONLY<<6) | (Directory::READ_ONLY<<3) |(Directory::READ_ONLY);
        m_handle = shm_open(name.c_str(), flag | (O_CREAT | O_EXCL), permission);
        if(m_handle >= 0)
        {
            ::fchmod(m_handle, permission);
            m_linked = true;
            return;
        }
        else if(errno == EEXIST)
        {
            m_handle = shm_open(name.c_str(), flag, permission);
            if(m_handle >= 0)
            {
                m_linked = true;
                return;
            }
        }
        PLATFORM_VERIFY(false);
    }
    
    void SharedObject::Unlink()
    {
       PLATFORM_VERIFY(::shm_unlink(m_name.c_str()) == 0);
       m_linked = false;
    }
    
    SharedObject::~SharedObject()
    {
        if(m_linked == true)
            Unlink();
    }
    
    std::string SharedObject::AddLeadingSlash(const std::string &name)
    {
        std::string slashedName;
        if(name.length() && name[0] != '/')
        {
            slashedName = "/" + name;
            return slashedName;
        }
        else
            return name;
        
    }
}
