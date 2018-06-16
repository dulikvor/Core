#pragma once

#include <string>
#include <memory>
#include "TraceListener.h"

namespace core
{
    class StdOutListener : public TraceListener
    {
    public:
        virtual ~StdOutListener(){}
        void Log(TraceSeverity severity, const std::string &msg) override;
        void Flush() override;
        void SetSeverity(TraceSeverity severity) override;

    private:
        TraceSeverity m_severity;
    };
}

