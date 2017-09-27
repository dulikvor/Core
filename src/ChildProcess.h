#pragma once

#include <memory>

namespace core {
    class Pipe;
}

namespace core{
    class ChildProcess {
    public:
        ChildProcess(int processID, std::unique_ptr<Pipe>& stdOutputPipe,
                     std::unique_ptr<Pipe>& stdErrorPipe);
        const Pipe& GetStdOutPipe(){return *m_stdOutput;}
        const Pipe& GetStdErrorPipe(){return *m_stdError;}

    private:
        int m_processID;
        std::unique_ptr<Pipe> m_stdOutput;
        std::unique_ptr<Pipe> m_stdError;
    };
}
