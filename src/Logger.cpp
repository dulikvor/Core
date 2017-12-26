
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
#include "Exception.h"
#include "TraceListener.h"
#include "Process.h"
#include "Environment.h"

using namespace std;
using namespace core;

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
        static_assert((int)TraceSeverity::Verbose == (int)level_enum::debug, "TraceSeverity::Verbose dosn't match spdlog debug");
        static_assert((int)TraceSeverity::Info == (int)level_enum::info, "TraceSeverity::Info dosn't match spdlog info");
        static_assert((int)TraceSeverity::Fatal == (int)level_enum::err, "TraceSeverity::Error dosn't match spdlog err");

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
            ASSERT(regex_search(buffer, fileNameLineMatch, fileNameLinePattern));

            static regex functionPattern("([a-zA-Z]*::[a-zA-Z]*)");
            cmatch functionMatch;
            if(unMangledName.get() != nullptr)
                ASSERT(regex_search(unMangledName.get(), functionMatch, functionPattern));

            return make_tuple(fileNameLineMatch[1].str(), fileNameLineMatch[2].str(), functionMatch.size() >= 1 ?
                      functionMatch[1].str() : "???????");

        }
#endif
        return make_tuple("???????", "???????", "???????");
    };

    void Logger::SetDefaultLogger() {

#ifdef SPDLOG_FOUND
        
        vector<shared_ptr<sink>> sinks;
        lock_guard<mutex> lock(m_mut);    
        {
            for(const shared_ptr<TraceListener>& listener : m_listeners)
            {
                sinks.emplace_back(listener->GetSink());
            }
        }
//        m_logger = make_shared<spdlog::async_logger>("Logger", sinks.begin(), sinks.end(), 4096,
//            spdlog::async_overflow_policy::discard_log_msg);
                    
        m_logger = make_shared<LoggerImpl>("Logger", sinks.begin(), sinks.end(), 4096,
            spdlog::async_overflow_policy::discard_log_msg);
#else
        m_logger = make_shared<LoggerImpl>();
#endif
    }

    void Logger::Start(TraceSeverity severity)
    {
        ASSERT(!m_running.exchange(true));
        if (!m_logger) {
            SetDefaultLogger();
        }
        
        m_logger->set_level(level_enum(severity));
        m_severity = severity;
    }
    void Logger::Log(TraceSeverity severity, const string& message)
    {
        m_logger->log((level_enum)severity, message.c_str());
        Flush();
    }

    void Logger::Flush()
    {
        m_logger->flush();
    }

    void Logger::AddListener(const shared_ptr<TraceListener>& listener)
    {
        lock_guard<mutex> lock(m_mut);
        m_listeners.emplace_back(listener);
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
        ASSERT(size != -1 && size < Local_buffer_size); //In windows version -1 is a legit answer
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
            ASSERT(largerSize != -1 && largerSize < bufferSize); //In windows version -1 is a legit answer
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
