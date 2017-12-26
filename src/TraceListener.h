#pragma once

#include <string>
#include <memory>
#include "Logger.h"

namespace spdlog
{
    namespace sinks
    {
        class sink;
    }
}

namespace core
{
    class TraceListener
    {
    public:
        virtual ~TraceListener(){}

    protected:
        std::shared_ptr<spdlog::sinks::sink> m_sink;

    private:
        friend Logger;
        //properties
        std::shared_ptr<spdlog::sinks::sink> GetSink(){ return m_sink; }
    };

    class FileRotationListener : public TraceListener
    {
    public:
        FileRotationListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount);
        virtual ~FileRotationListener(){}
    };
}
