#include "TraceListener.h"

using namespace std;

#ifdef SPDLOG_FOUND
#include "spdlog/sinks/file_sinks.h"
using namespace spdlog::sinks;
using namespace spdlog::level;
#endif

namespace core
{
    FileRotationListener::FileRotationListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount)
    {
#ifdef SPDLOG_FOUND
        m_sink = make_shared<rotating_file_sink_mt>(filePrefFix + ".log", maxFileSize, maxFilesCount);
#endif
    }
}
