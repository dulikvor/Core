#pragma once
#if defined(__linux)
#include <unistd.h>
#endif
#include "Assert.h"

namespace core{

    class Pipe {
    public:
        enum DEFUALT_DESCRIPTORS{
            STD_OUT = 1,
            STD_ERROR = 2
        };
    public:
        Pipe();
        ~Pipe();
        Pipe(const Pipe&) = delete;
        Pipe& operator=(const Pipe&) = delete;

        void CloseReadDescriptor();
        void CloseWriteDescriptor();
        int GetReadDescriptor() const { return m_fds[0]; }
        int GetWriteDescriptor() const { return m_fds[1]; }

    private:
#define PIPE_FD_NUM 2
        int m_fds[PIPE_FD_NUM];
        bool m_fdOpen[PIPE_FD_NUM];
    };

}
