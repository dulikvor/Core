#include "Pipe.h"
#include "Assert.h"

namespace core{
    Pipe::Pipe() {
#if defined (__linux)
        PLATFORM_VERIFY(pipe(m_fds) != -1);
#endif
        m_fdOpen[0] = true;
        m_fdOpen[1] = true;
    }

    Pipe::~Pipe(){
        if (m_fdOpen[0]){
#if defined (__linux)
            close(m_fds[0]);
#endif
        }
        if (m_fdOpen[1]){
#if defined (__linux)
            close(m_fds[1]);
#endif
        }
    }

    void Pipe::CloseReadDescriptor() {
#if defined (__linux)
        PLATFORM_VERIFY(close(m_fds[0]) == 0);
#endif
        m_fdOpen[0] = false;
    }

    void Pipe::CloseWriteDescriptor() {
#if defined (__linux)
        PLATFORM_VERIFY(close(m_fds[1]) == 0);
#endif
        m_fdOpen[1] = false;
    }
}


