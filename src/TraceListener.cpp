#include "TraceListener.h"
#include "spdlog/sinks/file_sinks.h"

using namespace std;
using namespace spdlog::sinks;
using namespace spdlog::level;

namespace core
{
    FileRotationListener::FileRotationListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount)
    {
        m_sink = make_shared<rotating_file_sink_mt>(filePrefFix + ".log", maxFileSize, maxFilesCount);
    }
}
