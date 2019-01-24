#include "Condition.h"
#include <limits>
#if defined(__linux)
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#endif

namespace core{
    
    Condition::Condition(): m_command(SLEEP), m_waitersCount(0){}
    
    void Condition::Signal(Command command)
    {
        m_wakeLock.lock();
        if(m_waitersCount == 0)
        {
            m_wakeLock.unlock();
            return;
        }
        m_command.exchange(command);
#if defined(__linux)
        syscall(SYS_futex, &m_command, FUTEX_WAKE, m_command == NOTIFY_ONE ? 1 : std::numeric_limits<int>::max(), nullptr, nullptr, 0);
#else
        throw Exception(__CORE_SOURCE, "wake is not being supported by current platform");
#endif
        
        while(m_command != SLEEP){}
        m_wakeLock.unlock();
        
    }
    
    void Condition::Wait(core::Mutex &mutex)
    {
        m_waitersCount++;
        mutex.unlock();
        while(true)
        {
            while(m_command == SLEEP)
#if defined(__linux)
                syscall(SYS_futex, &m_command, FUTEX_WAIT, SLEEP, nullptr, nullptr, 0);
#else
                throw Exception(__CORE_SOURCE, "wait is not being supported by current platform");
#endif
            
            int fromCommand_one = NOTIFY_ONE;
            if(m_command.compare_exchange_strong(fromCommand_one, static_cast<int>(SLEEP)))
            {
                m_waitersCount--;
                mutex.lock();
                return;
            }
            else if(m_command == NOTIFY_ALL)
            {
                if(m_waitersCount.fetch_sub(1) == 1)
                    m_command.exchange(SLEEP);
                
                mutex.lock();
                return;
            }
        }
    }
}
