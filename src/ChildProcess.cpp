#include "ChildProcess.h"
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
    
    ChildProcess::ChildProcess(core::ChildProcess &&object) NOEXCEPT
    : m_processID(object.m_processID), m_stdOutput(std::move(object.m_stdOutput)), m_stdError(std::move(object.m_stdError)){}
    
    ChildProcess& ChildProcess::operator=(core::ChildProcess &&rhs) NOEXCEPT
    {
        m_processID = rhs.m_processID;
        m_stdOutput = std::move(rhs.m_stdOutput);
        m_stdError = std::move(rhs.m_stdError);
        return *this;
    }
}

