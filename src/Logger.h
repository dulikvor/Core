#pragma once

#include <cstdarg>
#include <cassert>
#include <list>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <tuple>
#include <ctime>
#include <vector>
#include <algorithm>
#if defined(__linux)
#include <execinfo.h>
#endif
#include "Source.h"
#include "LoggerImpl.h"
#include "Export.h"

namespace core
{
    const int ORIGINAL_STACK_FRAMES_GUESS = 20; //20 is arbitrary
    class TraceListener;
    enum TraceSeverity : short
    {
        Verbose = 1,
        Info,
        Fatal = 4,
        NoneWorking = 5
    };

    class Logger
    {
    private:
        template<typename... Args>
        struct Comperator
        {
            static const bool value = sizeof...(Args) == 0;
        };
    public:
        CORE_EXPORT static Logger& Instance();
        CORE_EXPORT ~Logger(); //add nullptr to release the blocked wait. not by cancel of the thread.
        void AddListener(const std::shared_ptr<TraceListener>& listener);
        std::string BuildMessage(const Source& source, const char* format, ...);
        void SetImpl(std::unique_ptr<LoggerImpl> loggerImpl);
        CORE_EXPORT void Start(TraceSeverity severity);
        CORE_EXPORT void Terminate();
        template<typename... Args>
        void Trace(TraceSeverity severity, const Source& source, const char* format, Args... args)
        {
            ValidateParams<Args...>();
            std::string message = BuildMessage(source, format, args...);
            Log(severity, message.c_str());
        }
        template<typename... Args>
        static typename std::enable_if<Comperator<Args...>::value>::type ValidateParams(){} //Due to VS2013 limitation on expression SFIANE
        template<typename X, typename... Args>
        static void ValidateParams(){
            static_assert(std::is_same<X, std::string>::value == false, "Format only supports c-type string as type, don't use string");
            ValidateParams<Args...>();
        }

        void PrintStack()
        {
#if defined(__linux)
            //Trace the entire stack frames:
            //Get the stack frames data
            void** stackFramesAddresses = (void**)malloc(sizeof(void*)*ORIGINAL_STACK_FRAMES_GUESS);
            int stackFramesSize = ORIGINAL_STACK_FRAMES_GUESS;
            int readFramesCount;
            while(stackFramesSize == (readFramesCount = backtrace(stackFramesAddresses, stackFramesSize)))
            {
                free(stackFramesAddresses);
                stackFramesSize*=2;
                stackFramesAddresses = (void**)malloc(sizeof(void*)*stackFramesSize);
            }
            //Get the symbols
            char** stackFramesSymbols = backtrace_symbols(stackFramesAddresses, readFramesCount);
            for(int index = 0; index < readFramesCount; index++){
                auto frameInformation = GetFunctionAndLine(stackFramesSymbols[index]);
                Trace(TraceSeverity::Fatal, __CORE_SOURCE, "%s:%s:%s", std::get<0>(frameInformation).c_str(),
                      std::get<1>(frameInformation).c_str(), std::get<2>(frameInformation).c_str());
            }
            free(stackFramesAddresses);
            free(stackFramesSymbols);
#endif
        }
        CORE_EXPORT void Log(TraceSeverity severity, const char* message);
        CORE_EXPORT void Flush();
        CORE_EXPORT TraceSeverity GetSeverity() const { return m_severity; }

    private:
        using FunctionName = std::string;
        using Line = std::string;
        using FileName = std::string;

        Logger();
        std::tuple<FileName, Line, FunctionName> GetFunctionAndLine(char* mangledSymbol);
        void SetDefaultLogger();
        
    private:
        std::list<std::shared_ptr<TraceListener>> m_listeners;
        std::unique_ptr<LoggerImpl> m_loggerImpl;
        std::atomic_bool m_running;
        TraceSeverity m_severity;
        mutable std::mutex m_mut;
        static const int Local_buffer_size = 2000;
    };


    inline std::string Logger::BuildMessage(const Source& source, const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        std::string result;

        char buf[Local_buffer_size] = "";
#if defined(WIN32)
        int size = _snprintf_s(buf, Local_buffer_size,  Local_buffer_size - 1, "%s:%s:%d\t", source.file,
            source.function, source.line);
#else
        int size = snprintf(buf, Local_buffer_size, "%s:%s:%d\t", source.file, source.function, source.line);
        assert(size >= 0);
#endif
        assert(size != -1 && size < Local_buffer_size); //In windows version -1 is a legit answer
#if defined(WIN32)
        int tempSize = vsprintf_s(buf + size, Local_buffer_size - size, format, arguments);
        tempSize != -1 ? size += tempSize : size = -1;
#else
        int tempSize = vsnprintf(buf + size, Local_buffer_size - size, format, arguments);
        assert(tempSize >=0);
        size += tempSize;
#endif

        if(size != -1 && size < Local_buffer_size)
            result = buf;
        else //message was trunced or operation failed
        {
            int bufferSize = std::max<int>(size, 32 * 1024);
            std::vector<char> largerBuf;
            largerBuf.resize(bufferSize);
#if defined(WIN32)
            int largerSize = _snprintf_s(&largerBuf[0], bufferSize, bufferSize - 1, "%s:%s:%d\t", source.file, source.function, source.line);
#else
            int largerSize = snprintf(&largerBuf[0], bufferSize, "%s:%s:%d\t", source.file, source.function, source.line);
            assert(largerSize >= 0);
#endif
            assert(largerSize != -1 && largerSize < bufferSize); //In windows version -1 is a legit answer
#if defined(WIN32)
            vsprintf_s(&largerBuf[largerSize], bufferSize - largerSize, format, arguments); //We will print what we can, no second resize.
#else
            int remainSize = vsnprintf(&largerBuf[largerSize], bufferSize - largerSize, format, arguments);
            assert(remainSize >= 0); //We will print what we can, no second resize.
#endif
            result = std::string(largerBuf.begin(), largerBuf.end());
        }

        va_end(arguments);
        return result;
    }
}


#define TRACE_IMPL(severity, ...)\
    if(severity >= core::Logger::Instance().GetSeverity()) \
        core::Logger::Instance().Trace(severity, __CORE_SOURCE, __VA_ARGS__)

#define TRACE_ERROR(...) \
    TRACE_IMPL(core::TraceSeverity::Fatal, __VA_ARGS__)
#define TRACE_INFO(...) \
    TRACE_IMPL(core::TraceSeverity::Info, __VA_ARGS__)
#define TRACE_VERBOSE(...) \
    TRACE_IMPL(core::TraceSeverity::Verbose, __VA_ARGS__)
