#pragma once

#include "LoggerImpl.h"
#include "DefaultTraceListeners.h"

namespace core
{
    class DefaultLogger: public LoggerImpl
    {
    public:
        virtual ~DefaultLogger();
        void Start(TraceSeverity severity) override;
        void Log(TraceSeverity severity, const std::string& msg) override;
        void Flush() override;
        void AddListener(const std::shared_ptr<TraceListener>& listener) override;

    private:
        StdOutListener m_listener;
    };
}

