#include "DefaultLogger.h"
#include "Exception.h"

namespace core
{
    DefaultLogger::~DefaultLogger() {}

    void DefaultLogger::Start(TraceSeverity severity) {
        m_listener.SetSeverity(severity);
    }

    void DefaultLogger::Log(TraceSeverity severity, const std::string &msg) {
        m_listener.Log(severity, msg);
    }

    void DefaultLogger::Flush() {
        m_listener.Flush();
    }

    void DefaultLogger::AddListener(const std::shared_ptr<TraceListener> &listener) {
        throw Exception(__CORE_SOURCE, "Adding listeners to default logger is not possible.");
    }
}

