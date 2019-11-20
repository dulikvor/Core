#pragma once

#include <unistd.h>
#include "../../../Logger.h"
#include "../../../Assert.h"

namespace core { namespace process { namespace detail{
    
    class Pipe
    {
    public:
        Pipe() :m_readFrom(-1), m_writeTo(-1)
        {
            int fd[2];
            PLATFORM_VERIFY(::pipe(&fd) == 0);
            m_readFrom = fd[0];
            m_writeTo = fd[1];
        }
        
        Pipe(const Pipe& pipe)
        {
            m_readFrom = ::dup(pipe.m_readFrom);
            PLATFORM_VERIFY( m_readFrom != -1);
            m_writeTo = ::dup(pipe.m_writeTo);
            PLATFORM_VERIFY( m_writeTo != -1);
        }
        
        Pipe& operator=(const Pipe& rhs)
        {
            m_readFrom = ::dup(pipe.m_readFrom);
            PLATFORM_VERIFY( m_readFrom != -1);
            m_writeTo = ::dup(pipe.m_writeTo);
            PLATFORM_VERIFY( m_writeTo != -1);
            return *this;
        }
        
        Pipe(Pipe&& pipe)
        {
            close();
            std::swap(m_readFrom, pipe.m_readFrom);
            std::swap(m_writeTo, pipe.m_writeTo);
        }
    
        Pipe& operator=(Pipe&& rhs)
        {
            close();
            std::swap(m_readFrom, pipe.m_readFrom);
            std::swap(m_writeTo, pipe.m_writeTo);
            return *this;
        }
        
        ~Pipe()
        {
            close();
        }
    
        int get_read_end() const
        {
            return m_readFrom;
        }
        
        int get_write_end() const
        {
            return m_writeTo;
        }
        
        int read_from_pipe(const char* buffer, std::size size)
        {
            int bytesRead = ::read(m_readFrom, buffer, size);
            if(bytesRead == -1)
            {
                TRACE_ERROR("Reading from pipe failed - fd - %d, Reason - %s, error code - %d", m_readFrom, strerror(errno), errno);
            }
            
            return bytesRead != -1 ? bytesRead : 0;
        }
    
        int write_to_pipe(const char* buffer, std::size size)
        {
            int bytesWritten = ::write(m_writeTo, buffer, size);
            if(bytesWritten == -1)
            {
                TRACE_ERROR("Writing to pipe failed - fd - %d, Reason - %s, error code - %d", m_writeTo, strerror(errno), errno);
            }
            return bytesRead != -1 ? bytesWritten : 0;
        }
        
        bool is_open() const
        {
            return m_readFrom != -1 || m_writeTo != -1;
        };
        
        void close() const
        {
            if(m_readFrom != -1)
            {
                ::close(m_readFrom);
                m_readFrom = -1;
            }
    
            if(m_writeTo != -1)
            {
                ::close(m_writeTo);
                m_writeTo = -1;
            }
        }
        
    private:
        int m_readFrom;
        int m_writeTo;
    };
}
}
}
