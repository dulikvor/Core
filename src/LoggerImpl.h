#pragma once

#include <memory>
#include <string>

namespace core
{
    enum TraceSeverity : short;
    class TraceListener;

    class LoggerImpl
    {
    public:
        virtual ~LoggerImpl(){}
        virtual void Start(TraceSeverity) = 0;
        virtual void Log(TraceSeverity, const std::string&) = 0;
        virtual void Flush() = 0;
        virtual void AddListener(const std::shared_ptr<TraceListener>& listener) = 0;
    };
}
