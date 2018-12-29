#pragma once

#include <string>

namespace core
{
    class SharedRegion
    {
    public:
        SharedRegion();
        SharedRegion(void* base, int pageOffset, size_t size);
        SharedRegion(const SharedRegion& object);
        SharedRegion& operator=(const SharedRegion& rhs);
        char* GetPtr();
        size_t GetSize() const;
        void UnMap();
        
    private:
        void* m_base;
        char* m_ptr;
        size_t m_size;
        bool m_mapped;
    };
    class SharedObject
    {
    public:
        enum class AccessMod
        {
            READ_WRITE,
            READ
        };
        SharedObject(const std::string& name, AccessMod mod);
        void Allocate(size_t size);
        SharedRegion Map(int offset, size_t size, AccessMod mode);
        void Unlink();
        
    private:
        static std::string AddLeadingSlash(const std::string& name);
        int CorrectedPageOffset(int offset);
        
    private:
        const size_t m_pageSize;
        int m_handle;
        std::string m_name;
    };
}