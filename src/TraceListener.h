#pragma once

#include <string>

namespace core
{
    enum TraceSeverity : short;

    class TraceListener
    {
    public:
        virtual void Log(TraceSeverity severity, const std::string &msg) = 0;
        virtual void Flush() = 0;
        virtual void SetSeverity(TraceSeverity severity) = 0;
        virtual ~TraceListener() {}
    };
}
