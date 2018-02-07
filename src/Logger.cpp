#include <cassert>

#include <stdarg.h>
#if defined(WIN32)
#else
#include <cxxabi.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <regex>

#include "Logger.h"
#include "Assert.h"
#include "TraceListener.h"
#include "Process.h"
#include "Environment.h"

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
    }

    tuple<Logger::FileName, Logger::Line, Logger::FunctionName> Logger::GetFunctionAndLine(char* mangledSymbol){
#if defined(WIN32)
#else
        int status;
        static regex functionManglingPattern("\\((.*)\\+(0x[0-9a-f]*)\\)\\s*\\[(0x[0-9a-f]*)\\]");
        cmatch functionMangaledMatch;
        if(regex_search(mangledSymbol, functionMangaledMatch, functionManglingPattern)) {
            unique_ptr<char, void (*)(void *)> unMangledName(
                    abi::__cxa_demangle(functionMangaledMatch[1].str().c_str(), nullptr, 0, &status), &std::free);

            //Get file and line
            ChildProcess childProcess = Process::SpawnChildProcess("addr2line", "addr2line", (string("--exe=") + Environment::Instance().GetProcessPath().c_str()
                 + Environment::Instance().GetProcessName().c_str()).c_str(), functionMangaledMatch[3].str().c_str(), (const char*)NULL);

            char buffer[1024] = {0};
            PLATFORM_VERIFY(read(childProcess.GetStdOutPipe().GetReadDescriptor(),
                                          buffer, 1024) != -1);
            static regex fileNameLinePattern("([a-zA-Z0-9]*.[a-zA-Z0-9]*):([0-9]*)");
            cmatch fileNameLineMatch;
            assert(regex_search(buffer, fileNameLineMatch, fileNameLinePattern));

            static regex functionPattern("([a-zA-Z]*::[a-zA-Z]*)");
            cmatch functionMatch;
            if(unMangledName.get() != nullptr)
                assert(regex_search(unMangledName.get(), functionMatch, functionPattern));

            return make_tuple(fileNameLineMatch[1].str(), fileNameLineMatch[2].str(), functionMatch.size() >= 1 ?
                      functionMatch[1].str() : "???????");

        }
#endif
        return make_tuple("???????", "???????", "???????");
    }

    void Logger::SetImpl(unique_ptr<LoggerImpl> loggerImpl)
    {
        swap(m_loggerImpl, loggerImpl);
    }

    void Logger::Start(TraceSeverity severity)
    {
        assert(m_loggerImpl.get() != nullptr);
        assert(!m_running.exchange(true));
        m_loggerImpl->Start(severity);
        m_severity = severity;
    }
    void Logger::Log(TraceSeverity severity, const string& message)
    {
        assert(m_loggerImpl.get() != nullptr);
        m_loggerImpl->Log(severity, message.c_str());
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

    string Logger::BuildMessage(const Source& source, const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        string result;

        char buf[Local_buffer_size] = "";
#if defined(WIN32)
        int size = _snprintf(buf, Local_buffer_size, "%s:%s:%d\t", source.file,
            source.function, source.line);
#else
        int size;
        PLATFORM_VERIFY(size = snprintf(buf, Local_buffer_size, "%s:%s:%d\t", source.file, source.function, source.line) >= 0);
#endif
        assert(size != -1 && size < Local_buffer_size); //In windows version -1 is a legit answer
#if defined(WIN32)
        int tempSize = vsnprintf(buf + size, Local_buffer_size - size, format, arguments);
        tempSize != -1 ? size += tempSize : size = -1;
#else
        int tempSize;
        PLATFORM_VERIFY(tempSize = vsnprintf(buf + size, Local_buffer_size - size, format, arguments) >= 0);
        size += tempSize;
#endif
        
        if(size != -1 && size < Local_buffer_size)
            result = buf;
        else //message was trunced or operation failed
        {
            int bufferSize = max(size, 32 * 1024);
            vector<char> largerBuf;
            largerBuf.resize(bufferSize);
            
#if defined(WIN32)
            int largerSize = _snprintf(&largerBuf[0], bufferSize, "%s:%s:%d\t", source.file, source.function, source.line);
#else
            int largerSize;
            PLATFORM_VERIFY(largerSize = snprintf(&largerBuf[0], bufferSize, "%s:%s:%d\t", source.file, source.function, source.line) >= 0);
#endif
            assert(largerSize != -1 && largerSize < bufferSize); //In windows version -1 is a legit answer
#if defined(WIN32)
            vsnprintf(&largerBuf[largerSize], bufferSize - largerSize, format, arguments); //We will print what we can, no second resize.
#else
            PLATFORM_VERIFY(vsnprintf(&largerBuf[largerSize], bufferSize - largerSize, format, arguments) >= 0); //We will print what we can, no second resize.
#endif
            result = string(largerBuf.begin(), largerBuf.end());
        }

        va_end(arguments);
        return result;
    }
}
