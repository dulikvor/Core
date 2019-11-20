#pragma once

#include <streambuf>
#include <array>
#if defined(WIN32)
#else
#include "Posix/Pipe.h"
#endif

namespace core{ namespace process{ namespace detail{
    
    template<std::size_t BufferSize>
    class PipeBuffer : public std::streambuf
    {
    public:
        PipeBuffer() :m_readBuffer(BufferSize), m_writeBuffer(BufferSize)
        {
            setg(m_readBuffer.begin(), m_readBuffer.end(), m_readBuffer.end());
            setp(m_writeBuffer.begin(), m_writeBuffer.end());
        }
        
        const Pipe& get_pipe() const { return m_pipe; }
        
        int_type underflow() override
        {
            if(eback() == gptr()) //full, no space in the input buffer
                return static_cast<int_type>(*gptr());
            
            int emptySpace = gptr() - eback();
            int readBytes = m_pipe.read_from_pipe(gptr(), emptySpace);
            if(readBytes == 0)
                traits_type::eof();
            
            _M_in_cur -= readBytes;
            return static_cast<int_type>(*gptr());
        }
        
        int_type overflow(int_type __c  = traits_type::eof()) override
        {
            int occupiedSpace = pptr() - pbase();
            int bytesWritten = m_pipe.write_to_pipe(pptr(), occupiedSpace);
            if(bytesWritten == 0)
                return traits_type::eof();
            
            _M_out_cur += bytesWritten;
            return static_cast<int_type>(*pptr());
        }
        
    private:
        std::array<char, BufferSize> m_readBuffer;
        std::array<char, BufferSize> m_writeBuffer;
        Pipe m_pipe;
    };
    
}}}