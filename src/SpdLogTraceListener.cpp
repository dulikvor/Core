#include "SpdLogTraceListener.h"
#include "spdlog/sinks/file_sinks.h"
#include "Exception.h"

namespace core
{
    void FileRotationTraceListener::Log(TraceSeverity severity, const std::string &msg)
    {
        throw Exception(__CORE_SOURCE, "FileRotationTraceListener does not support addressing Log directly.");
    }

    void FileRotationTraceListener::Flush()
    {
        throw Exception(__CORE_SOURCE, "FileRotationTraceListener does not support addressing Flush directly.");
    }

    void FileRotationTraceListener::SetSeverity(TraceSeverity severity)
    {
        throw Exception(__CORE_SOURCE, "FileRotationTraceListener does not support addressing SetSeverity directly.");
    }

    FileRotationTraceListener::FileRotationTraceListener(TraceSeverity severity, const std::string& filePrefFix, int maxFileSize, int maxFilesCount)
    {
        m_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePrefFix + ".log", maxFileSize, maxFilesCount);
    }
}
