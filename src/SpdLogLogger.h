#pragma once

#include <vector>
#include <mutex>
#include "LoggerImpl.h"

namespace spdlog
{
    class logger;
}

namespace core
{
    class SpdLogLogger: public LoggerImpl
    {
    public:
        SpdLogLogger();
        ~SpdLogLogger();
        void Start(TraceSeverity severity) override;
        void Log(TraceSeverity severity, const std::string& msg) override;
        void Flush() override;
        void AddListener(const std::shared_ptr<TraceListener>& listener) override;

    private:
        std::vector<std::shared_ptr<TraceListener>> m_listeners;
        std::shared_ptr<spdlog::logger> m_logger;
        mutable std::mutex m_mut;
    };
}
