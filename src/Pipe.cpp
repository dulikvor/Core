#include "Pipe.h"
#include "Assert.h"

namespace core{
    Pipe::Pipe() {
        LINUX_VERIFY(pipe(m_fds) != -1);
        m_fdOpen[0] = true;
        m_fdOpen[1] = true;
    }

    Pipe::~Pipe(){
        if(m_fdOpen[0])
            LINUX_VERIFY(close(m_fds[0]) == 0);
        if(m_fdOpen[1])
            LINUX_VERIFY(close(m_fds[1]) == 0);
    }

    void Pipe::CloseReadDescriptor() {
        LINUX_VERIFY( close(m_fds[0]) == 0);
        m_fdOpen[0] = false;
    }

    void Pipe::CloseWriteDescriptor() {
        LINUX_VERIFY( close(m_fds[1]) == 0);
        m_fdOpen[1] = false;
    }
}


