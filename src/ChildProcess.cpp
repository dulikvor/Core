#include "ChildProcess.h"
#if defined(__linux)
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include "Pipe.h"
#include "Assert.h"

using namespace std;

namespace core{
    ChildProcess::ChildProcess(int processID, unique_ptr<Pipe>& stdOutputPipe,
      unique_ptr<Pipe>& stdErrorPipe): m_processID(processID),
          m_stdOutput(move(stdOutputPipe)), m_stdError(move(stdErrorPipe)){
        m_stdOutput->CloseWriteDescriptor();
        m_stdError->CloseWriteDescriptor();
    }
    
    ChildProcess::ChildProcess(core::ChildProcess &&object) NOEXCEPT(true)
    : m_processID(object.m_processID), m_stdOutput(std::move(object.m_stdOutput)), m_stdError(std::move(object.m_stdError)){}
    
    ChildProcess& ChildProcess::operator=(core::ChildProcess &&rhs) NOEXCEPT(true)
    {
        m_processID = rhs.m_processID;
        m_stdOutput = std::move(rhs.m_stdOutput);
        m_stdError = std::move(rhs.m_stdError);
        return *this;
    }
    
    void ChildProcess::wait()
    {
#if defined(__linux)
        int status;
        ::waitpid(m_processID, &status, 0);
        VERIFY(WIFEXITED(status), "Process %d didn't exit sucesfully", m_processID);
#endif
    }
}

