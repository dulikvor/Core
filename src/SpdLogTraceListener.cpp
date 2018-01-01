#include "SpdLogTraceListener.h"
#include "spdlog/sinks/file_sinks.h"

using namespace std;

namespace core
{
    FileRotationTraceListener::FileRotationTraceListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount)
    {
        m_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(filePrefFix + ".log", maxFileSize, maxFilesCount);
    }
}
