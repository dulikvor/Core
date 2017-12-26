#pragma once

#include <string>
#include <memory>
#include "Logger.h"

#ifdef SPDLOG_FOUND

namespace spdlog
{
    namespace sinks
    {
        class sink;
    }
}

#endif

namespace core
{
    class TraceListener
    {
    public:
        virtual ~TraceListener(){}

    protected:
        
#ifdef SPDLOG_FOUND
        std::shared_ptr<spdlog::sinks::sink> m_sink;
        std::shared_ptr<spdlog::sinks::sink> GetSink(){ return m_sink; }
#endif

    private:
        friend Logger;

    };

    class FileRotationListener : public TraceListener
    {
    public:
        FileRotationListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount);
        virtual ~FileRotationListener(){}
    };
}
