#include "SharedObject.h"
#include <iostream>
#if defined(__linux)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm-generic/errno-base.h>
#endif
#include "Exception.h"
#include "Directory.h"
#include "Assert.h"

namespace core{
    
    SharedRegion::SharedRegion()
        :m_base(nullptr), m_ptr(nullptr), m_size(0), m_mapped(false){}
    
    SharedRegion::SharedRegion(void *base, int pageOffset, size_t size)
        :m_base(base), m_ptr(reinterpret_cast<char*>(base) + pageOffset), m_size(size), m_mapped(true){}
    
    SharedRegion::SharedRegion(const SharedRegion& object)
        :m_base(object.m_base), m_ptr(object.m_ptr), m_size(object.m_size), m_mapped(object.m_mapped){}
        
    SharedRegion& SharedRegion::operator=(const SharedRegion& rhs)
    {
       m_base = rhs.m_base;
       m_ptr = rhs.m_ptr;
       m_size = rhs.m_size;
       m_mapped = rhs.m_mapped;
       return *this;
    }
    
    char* SharedRegion::GetPtr() {return m_ptr;}
    size_t SharedRegion::GetSize() const {return m_size;}
    
    void SharedRegion::UnMap()
    {
        if(m_mapped)
        {
            #if defined(__linux)
            PLATFORM_VERIFY(::munmap(m_base, m_size) == 0);
            m_mapped = false;
            #endif
        }
    }
    
    SharedObject::SharedObject(const std::string &name, AccessMod mod)
    #if defined(__linux)
        :m_pageSize(sysconf(_SC_PAGESIZE)), m_handle(-1)
    #else
        :m_pageSize(0), m_handle(-1)
    #endif
    
    {
        #if defined(__linux)
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
            return;
        }
        else if(errno == EEXIST)
        {
            m_handle = shm_open(name.c_str(), flag, permission);
            if(m_handle >= 0)
                return;
        }
        PLATFORM_VERIFY(false);
        #else
        throw Exception(__CORE_SOURCE, "SharedObject is only supported in linux platform");
        #endif
    }
    
    void SharedObject::Allocate(size_t size)
    {
        #if defined(__linux)
        VERIFY(m_handle != -1, "SharedObject is not opened/created yet");
        PLATFORM_VERIFY(::ftruncate(m_handle, size) == 0);
        #endif
    }
    
    void SharedObject::Unlink()
    {
       #if defined(__linux)
       int ret = ::shm_unlink(m_name.c_str());
       if(ret == 0 || (ret == -1 && errno == ENOENT))
       {
           return;
       }
       PLATFORM_VERIFY(false);
       #endif
    }
    
    SharedRegion SharedObject::Map(int offset, size_t size, AccessMod mod)
    {
        #if defined(__linux)
        int pageOffset = CorrectedPageOffset(offset);
        int prot = 0;
        switch(mod)
        {
            case AccessMod::READ:
                prot = PROT_READ;
                break;
            case AccessMod::READ_WRITE:
                prot = (PROT_READ | PROT_WRITE);
                break;
            default:
                throw Exception(__CORE_SOURCE, "Non supported mod");
        }
    
        void* base = ::mmap(NULL, static_cast<std::size_t>(pageOffset + size), prot, MAP_SHARED, m_handle, offset - pageOffset);
        PLATFORM_VERIFY(base != (void*)-1);
        return SharedRegion(base, pageOffset, size);
        #else
        return SharedRegion(nullptr, -1, -1);
        #endif
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
    
    int SharedObject::CorrectedPageOffset(int offset)
    {
        return offset % m_pageSize;
    }
}
