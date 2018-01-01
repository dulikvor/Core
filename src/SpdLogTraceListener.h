#pragma once

#include <string>
#include <memory>
#include "TraceListener.h"

namespace spdlog { namespace sinks {
        class sink;
    }
}

namespace core
{
    class SpdLogLogger;
    enum TraceSeverity : short;

    class SpdLogTraceListener : public TraceListener
    {
    public:
        virtual ~SpdLogTraceListener(){}
    protected:
        std::shared_ptr<spdlog::sinks::sink> m_sink;
        std::shared_ptr<spdlog::sinks::sink> GetSink(){ return m_sink; }

    private:
        friend SpdLogLogger;
    };

    class FileRotationTraceListener : public SpdLogTraceListener
    {
    public:
        FileRotationTraceListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount);
        virtual ~FileRotationTraceListener(){}
    };
}
