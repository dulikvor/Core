#include "SpdLogLogger.h"
#include <vector>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include "SpdLogTraceListener.h"
#include "Logger.h"

using namespace std;

namespace core
{
    SpdLogLogger::SpdLogLogger() {
        static_assert((int)TraceSeverity::Verbose == (int)spdlog::level::level_enum::debug, "TraceSeverity::Verbose dosn't match spdlog debug");
        static_assert((int)TraceSeverity::Info == (int)spdlog::level::level_enum::info, "TraceSeverity::Info dosn't match spdlog info");
        static_assert((int)TraceSeverity::Fatal == (int)spdlog::level::level_enum::err, "TraceSeverity::Error dosn't match spdlog err");
    }

    SpdLogLogger::~SpdLogLogger() {}

    void SpdLogLogger::Start(TraceSeverity severity) {
        vector<shared_ptr<spdlog::sinks::sink>> sinks;
        lock_guard<mutex> lock(m_mut);
        {
            for(const shared_ptr<TraceListener>& listener : m_listeners)
            {
                SpdLogTraceListener* spdLogListener = reinterpret_cast<SpdLogTraceListener*>(listener.get());
                sinks.emplace_back(spdLogListener->GetSink());
            }
        }
        m_logger = make_shared<spdlog::async_logger>("Logger", sinks.begin(), sinks.end(), 4096,
                                           spdlog::async_overflow_policy::discard_log_msg);

        m_logger->set_level(spdlog::level::level_enum(severity));
    }

    void SpdLogLogger::Log(TraceSeverity severity, const string &msg) {
        m_logger->log((spdlog::level::level_enum)severity, msg.c_str());
    }

    void SpdLogLogger::Flush() {
        m_logger->flush();
    }

    void SpdLogLogger::AddListener(const shared_ptr<TraceListener> &listener) {
        lock_guard<mutex> lock(m_mut);
        m_listeners.emplace_back(listener);
    }
}
