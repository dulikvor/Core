#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include "SharedObject.h"
#include "Mutex.h"
#include "Condition.h"

namespace core{
    
    template<typename Type, std::size_t Count>
    class SWSRCyclicBuffer
    {
    public:
        SWSRCyclicBuffer():m_readIdx(0), m_writeIdx(0){}
        bool is_empty() const { return m_readIdx == m_writeIdx; }
        bool is_full() const { return (m_writeIdx + 1) % Count == m_readIdx; } //we are going to loose one write
        
        bool write(Type&& elem)
        {
            if(is_full() == false)
            {
                m_buffer[m_writeIdx] = std::move(elem);
                m_writeIdx = (++m_writeIdx) % Count;
                return true;
            }
            return false;
        }
    
        bool write(Type& elem)
        {
            if(is_full() == false)
            {
                m_buffer[m_writeIdx] = elem;
                m_writeIdx = (++m_writeIdx) % Count;
                return true;
            }
            return false;
        }
        
        template<typename X = Type>
        typename std::enable_if<std::is_move_assignable<X>::value == true, bool>::type read(Type& elem)
        {
            if(is_empty() == false)
            {
                elem = std::move(m_buffer[m_readIdx]);
                m_readIdx = (++m_readIdx) % Count;
                return true;
            }
            return false;
        }
    
        template<typename X = Type>
        typename std::enable_if<std::is_move_assignable<X>::value == false, bool>::type read(Type& elem)
        {
            if(is_empty() == false)
            {
                elem = m_buffer[m_readIdx];
                m_readIdx = (++m_readIdx) % Count;
                return true;
            }
            return false;
        }
        
    private:
        int m_readIdx;
        int m_writeIdx;
        bool m_full;
        bool m_empty;
        Type m_buffer[Count];
    };
    
    template<typename Type, std::size_t Count>
    class SyncSharedQueue
    {
    public:
        SyncSharedQueue(const std::string& name, bool owner, SharedObject::AccessMod mod)
            :m_sharedObject(name, mod), m_owner(owner)
        {
            std::size_t chunkSize = sizeof(SWSRCyclicBuffer<Type, Count>) + sizeof(Mutex) + 2*sizeof(ConditionVariable);
            m_sharedObject.Allocate(chunkSize);
            m_region = m_sharedObject.Map(0, chunkSize, mod);
            if(m_owner)
            {
                m_buffer = new(m_region.GetPtr())SWSRCyclicBuffer<Type, Count>();
                m_mutex = new(reinterpret_cast<char*>(m_buffer) + sizeof(SWSRCyclicBuffer<Type, Count>))Mutex();
                m_cvFull = new(reinterpret_cast<char*>(m_mutex) + sizeof(Mutex))ConditionVariable();
                m_cvEmpty = new(reinterpret_cast<char*>(m_cvFull) + sizeof(ConditionVariable))ConditionVariable();
            }
            else
            {
                m_buffer = reinterpret_cast<SWSRCyclicBuffer<Type, Count>*>(m_region.GetPtr());
                m_mutex = reinterpret_cast<Mutex*>(reinterpret_cast<char*>(m_buffer) + sizeof(SWSRCyclicBuffer<Type, Count>));
                m_cvFull = reinterpret_cast<ConditionVariable*>(reinterpret_cast<char*>(m_mutex) + sizeof(Mutex));
                m_cvEmpty = reinterpret_cast<ConditionVariable*>(reinterpret_cast<char*>(m_cvFull) + sizeof(ConditionVariable));
            }
        }
        
        ~SyncSharedQueue()
        {
            if(m_owner)
            {
                m_buffer->~SWSRCyclicBuffer<Type,Count>();
                m_mutex->~Mutex();
                m_cvFull->~ConditionVariable();
                m_cvEmpty->~ConditionVariable();
            }
            
            m_region.UnMap();
            m_sharedObject.Unlink();
        }
        
        template<typename ElementType>
        bool try_push(ElementType&& element)
        {
            std::lock_guard<Mutex> lock(*m_mutex);
            if(m_buffer->write(std::forward<ElementType>(element)) == false)
                return false;
            m_cvEmpty->notify_one();
            return true;
        }
    
        template<typename ElementType>
        bool try_pop(ElementType& element)
        {
            std::lock_guard<Mutex> lock(*m_mutex);
            if(m_buffer->read(element) == false)
                return false;
            m_cvFull->notify_one();
            return true;
        }
    
        template<typename ElementType>
        void push(ElementType&& element)
        {
            std::lock_guard<Mutex> lock(*m_mutex);
            m_cvFull->wait([&element, this]{return m_buffer->write(std::forward<ElementType>(element));}, *m_mutex);
            m_cvEmpty->notify_one();
        }
        
        template<typename ElementType>
        void pop(ElementType& element)
        {
            std::lock_guard<Mutex> lock(*m_mutex);
            m_cvEmpty->wait([&element, this]{return m_buffer->read(element);}, *m_mutex);
            m_cvFull->notify_one();
        }
        
    private:
        SharedObject m_sharedObject;
        SharedRegion m_region;
        SWSRCyclicBuffer<Type, Count>* m_buffer;
        Mutex* m_mutex;
        ConditionVariable* m_cvFull;
        ConditionVariable* m_cvEmpty;
        bool m_owner;
    };
}
