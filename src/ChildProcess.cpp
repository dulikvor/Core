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
}

