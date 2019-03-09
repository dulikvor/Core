#pragma once

#include <memory>
#include <utility>
#include <unordered_map>
#include <cmath>
#include <cstring>
#include <bitset>
#include "SharedObject.h"
#include "SymbolSet.h"

namespace core
{
    template<typename T> class AllocatorImpl;
    template<typename T> class BuddySharedAllocator;
    
    struct HeapType
    {
        enum Enumeration
        {
            Shared,
            Local
        };
    };
    
    template<typename T>
    class Allocator
    {
    public:
        typedef T* pointer;
        typedef size_t size_type;
        typedef T value_type;
        
        template<typename T1>
        Allocator& operator=(const Allocator<T1>& object)
        {
            switch(m_type)
            {
                case HeapType::Shared:
                    m_impl.reset(new BuddySharedAllocator<T1>());
                    *m_impl = *object.m_impl;
                    break;
                case HeapType::Local:
                    m_impl.reset(nullptr);
                    break;
                default:
                    throw Exception(__CORE_SOURCE, "Non supported heap type was provided - %d", static_cast<int>(m_type));
            }
            return *this;
        }
        
        template<typename T1>
        struct rebind
        {
            typedef Allocator<T1> other;
        };
        
        template<typename... Args>
        explicit Allocator(HeapType::Enumeration type, Args&&... args)
            :m_type(type)
        {
            switch(m_type)
            {
                case HeapType::Shared:
                    m_impl.reset(new BuddySharedAllocator<T>(std::forward<Args>(args)...));
                    return;
                case HeapType::Local:
                    m_impl.reset(nullptr);
                    return;
                default:
                    throw Exception(__CORE_SOURCE, "Non supported heap type was provided - %d", static_cast<int>(m_type));
            }
        }
        pointer allocate(size_type n, const void * hint = nullptr )
        {
            return m_impl->allocate(n*sizeof(value_type), hint);
        }
        void deallocate(pointer p, size_type n = 0)
        {
            m_impl->deallocate(p, n);
        }
        
    private:
        std::unique_ptr<AllocatorImpl<T>> m_impl;
        HeapType::Enumeration m_type;
    };
    
    template<typename T>
    class AllocatorImpl
    {
    public:
        typedef T* pointer;
        typedef size_t size_type;
        const int min_size = sizeof(T);
        
        virtual ~AllocatorImpl() = default;
        
        virtual pointer allocate(size_type n, const void * hint) = 0;
        virtual void deallocate(pointer p, size_type n) = 0;
    };
    
    class BuddyCell
    {
    public:
        static unsigned int CellsUpUntilCurrentLevel(unsigned int cellLevel);
        static unsigned int CellsPerLevel(unsigned int cellLevel);
        static char* CalculateCellAddress(char* const base, unsigned int cellLevel, unsigned int logarithmVal, unsigned int cell);
        static unsigned int AddressToCellIdx(unsigned int cellLevel, unsigned int logarithmVal, char* base, char *address);
        static unsigned int CalculateCellToLevel(unsigned int cell);
    };
    
    class BuddyTree
    {
    public:
        typedef std::size_t size_type;
        
        BuddyTree(unsigned int numCellsLevel, char* const buffer);
        char* Allocate(unsigned int cellLevelGuess);
        void Deallocate(char* address);
        
    private:
        enum BlockStatus : char
        {
            NILL = 0,
            NotAllocated = 1,
            Allocated = 2,
        };
        #define BLOCK_STATUS_BIT_COUNT 2
        static const unsigned int NON;
        
        void MergeCells(unsigned int startingCell);
        unsigned int ParitionCells(unsigned int startingCellLevel, unsigned int requestedCellLevel);
        void DecreaseCellLevel(unsigned int cellLevel, int count);
        void IncreaseCellLevel(unsigned int cellLevel, int count);
        
    private:
        std::unique_ptr<Symbolset<BLOCK_STATUS_BIT_COUNT>> m_buddyTree;
        std::unordered_map<unsigned int, int> m_availableCells;
        unsigned int m_topCellLevel;
        char* const m_buffer;
    };
    
    template<typename T>
    class BuddySharedAllocator : public AllocatorImpl<T>
    {
    public:
        typedef AllocatorImpl<T> base;
        typedef typename base::size_type size_type;
        typedef typename base::pointer pointer;
        
        BuddySharedAllocator()
            :m_owner(false)
        {
        }
        
        BuddySharedAllocator(const std::string& name, std::ptrdiff_t offset, int chunkSize = 1024 * 1024)
            :m_sharedObject(new SharedObject(name, SharedObject::AccessMod::READ_WRITE)), m_owner(true)
        {
            m_sharedObject->Allocate(chunkSize);
            m_region = m_sharedObject->Map(offset, chunkSize - offset, SharedObject::AccessMod::READ_WRITE);
            unsigned int cellLevel = ceil(log2(chunkSize - offset));
            m_buddyTree.reset(new BuddyTree(cellLevel, m_region.GetPtr()));
        }
    
        explicit BuddySharedAllocator(char* const buffer, int chunkSize = 1024 * 1024)
            :m_owner(false)
        {
            unsigned int cellLevel = ceil(log2(chunkSize));
            m_buddyTree.reset(new BuddyTree(cellLevel, buffer));
        }
        
        template<typename T1>
        BuddySharedAllocator& operator=(const BuddySharedAllocator<T1>& object)
        {
            m_sharedObject = object.m_sharedObject;
            m_region = object.m_region;
            m_buddyTree = object.m_buddyTree;
            m_owner = false;
            return *this;
        }
        
        virtual ~BuddySharedAllocator()
        {
            if(m_owner)
            {
                m_region.UnMap();
                m_sharedObject->Unlink();
            }
        }
        
        pointer allocate(size_type n, const void * hint) override
        {
            size_type requestedSize = n;
            unsigned int logarithmVal = ceil(log2(requestedSize));
            return reinterpret_cast<pointer>(m_buddyTree->Allocate(logarithmVal));
        }
    
        void deallocate(pointer p, size_type n) override
        {
            m_buddyTree->Deallocate(reinterpret_cast<char*>(p));
        }
        
    private:
        std::shared_ptr<BuddyTree> m_buddyTree;
        std::shared_ptr<SharedObject> m_sharedObject;
        SharedRegion m_region;
        mutable bool m_owner;
    };
}