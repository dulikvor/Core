#pragma once

#include <list>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <tuple>
#include <ctime>
#if defined(__linux)
#include <execinfo.h>
#endif
#include "Source.h"

namespace spdlog
{
    class logger;
}
namespace core
{
    class TraceListener;
}

enum TraceSeverity
{
    Verbose = 1,
    Info,
    Fatal = 4,
    NoneWorking = 5
};

const int ORIGINAL_STACK_FRAMES_GUESS = 20; //20 is arbitrary

namespace core
{
    class Logger
    {
    private:
        template<typename... Args>
        struct Comperator
        {
            static const bool value = sizeof...(Args) == 0;
        };
    public:
        static Logger& Instance();
        ~Logger(); //add nullptr to release the blocked wait. not by cancel of the thread.
        //Receives a new listener, a reference to the listener will be kept under the logger.
        void AddListener(const std::shared_ptr<TraceListener>& listener);
        //Builds a formatted message from a received ansi like format, matching arguments and 
        //the print source location. all will be used to genreate a string instance containing
        //the final result.
        std::string BuildMessage(const Source& source, const char* format, ...);
        //
        void Start(TraceSeverity severity);
        //
        template<typename... Args>
        void Trace(TraceSeverity severity, const Source& source, const char* format, Args... args)
        {
            ValidateParams<Args...>();
            std::string message = BuildMessage(source, format, args...);
            Log(severity, message);
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
                Trace(TraceSeverity::Fatal, SOURCE, "%s:%s:%s", std::get<0>(frameInformation).c_str(),
                      std::get<1>(frameInformation).c_str(), std::get<2>(frameInformation).c_str());
            }
            free(stackFramesAddresses);
            free(stackFramesSymbols);
#endif
        }
        //
        void Log(TraceSeverity severity, const std::string& message);
        //Flush the log.
        void Flush();
        //Properties
        TraceSeverity GetSeverity() const { return m_severity; }

    private:
        using FunctionName = std::string;
        using Line = std::string;
        using FileName = std::string;

        Logger();
        std::tuple<FileName, Line, FunctionName> GetFunctionAndLine(char* mangledSymbol);

    private:
        std::list<std::shared_ptr<TraceListener>> m_listeners;
        std::shared_ptr<spdlog::logger> m_logger;
        std::atomic_bool m_running;
        TraceSeverity m_severity;
        mutable std::mutex m_mut;
        static const int Local_buffer_size = 2000;
    };
}

#define TRACE_IMPL(severity, format, ...)\
    if(severity >= core::Logger::Instance().GetSeverity()) \
        core::Logger::Instance().Trace(severity, SOURCE, format, ##__VA_ARGS__)


#define TRACE_ERROR(format, ...) \
    TRACE_IMPL(TraceSeverity::Fatal, format, ##__VA_ARGS__)
#define TRACE_INFO(format, ...) \
    TRACE_IMPL(TraceSeverity::Info, format, ##__VA_ARGS__)
#define TRACE_VERBOSE(format, ...) \
    TRACE_IMPL(TraceSeverity::Verbose, format, ##__VA_ARGS__)
