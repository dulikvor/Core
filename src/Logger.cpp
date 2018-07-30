#if defined(WIN32)
#else
#include <cxxabi.h>
#endif
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <regex>

#include "Logger.h"
#include "TraceListener.h"
#include "Process.h"
#include "Environment.h"
#include "DefaultLogger.h"

using namespace std;

namespace core
{

    Logger& Logger::Instance()
    {
        static Logger logger;
        return logger;
    }

    Logger::~Logger()
    {
    }

    Logger::Logger(): m_severity(TraceSeverity::NoneWorking)
    {
        Environment::Instance().Init();
    }

    tuple<Logger::FileName, Logger::Line, Logger::FunctionName> Logger::GetFunctionAndLine(char* mangledSymbol)
    {
        static std::string unknown("???????");
#if defined(WIN32)
#else
        int status;
        static regex functionManglingPattern("\\((.*)\\+(0x[0-9a-f]*)\\)\\s*\\[(0x[0-9a-f]*)\\]");
        cmatch functionMangaledMatch;
        if(regex_search(mangledSymbol, functionMangaledMatch, functionManglingPattern)) {
            std::string functionMangaledName = functionMangaledMatch[1].str();
            unique_ptr<char, void (*)(void *)> unMangledName(
                    abi::__cxa_demangle(functionMangaledName.c_str(), nullptr, 0, &status), &std::free);

            //Get file and line
            ChildProcess childProcess = Process::SpawnChildProcess("addr2line", "addr2line", (string("--exe=") + Environment::Instance().GetProcessPath().c_str()
                 + Environment::Instance().GetProcessName().c_str()).c_str(), functionMangaledMatch[3].str().c_str(), (const char*)NULL);

            char buffer[1024] = {0};
            PLATFORM_VERIFY(read(childProcess.GetStdOutPipe().GetReadDescriptor(),
                                          buffer, 1024) != -1);
            if( strlen(buffer) > 0)
            {
                static regex fileNameLinePattern("([a-zA-Z0-9]*.[a-zA-Z0-9]*):([0-9]*)");
                cmatch fileNameLineMatch;
                assert(regex_search(buffer, fileNameLineMatch, fileNameLinePattern));
                std::string fileName = fileNameLineMatch.size() > 1 ? fileNameLineMatch[1].str() : std::string();
                std::string line = fileNameLineMatch.size() > 2 ? fileNameLineMatch[2].str() : std::string();
                return make_tuple( fileName.size() ? fileName : unknown, line.size() ? line : unknown, unMangledName && strlen(unMangledName.get()) > 0 ? std::string(unMangledName.get()) : functionMangaledName);
            }

            return make_tuple(unknown, unknown, unMangledName && strlen(unMangledName.get()) > 0 ? std::string(unMangledName.get()) : functionMangaledName);

        }
#endif
        return make_tuple(unknown, unknown, unknown);
    }

    void Logger::SetImpl(unique_ptr<LoggerImpl> loggerImpl)
    {
        swap(m_loggerImpl, loggerImpl);
    }

    void Logger::Start(TraceSeverity severity)
    {
        if(m_loggerImpl.get() == nullptr)
            m_loggerImpl.reset(new DefaultLogger());
        assert(!m_running.exchange(true, std::memory_order_relaxed));
        m_loggerImpl->Start(severity);
        m_severity = severity;
    }
    void Logger::Log(TraceSeverity severity, const char* message)
    {
        assert(m_loggerImpl.get() != nullptr);
        m_loggerImpl->Log(severity, message);
    }

    void Logger::Flush()
    {
        assert(m_loggerImpl.get() != nullptr);
        m_loggerImpl->Flush();
    }

    void Logger::AddListener(const shared_ptr<TraceListener>& listener)
    {
        assert(m_loggerImpl.get() != nullptr);
        m_loggerImpl->AddListener(listener);
    }

    void Logger::Terminate()
    {
        m_loggerImpl.release();
    }

}
