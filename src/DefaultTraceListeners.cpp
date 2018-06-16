#include "DefaultTraceListeners.h"
#include <iostream>

namespace core
{
    void StdOutListener::Log(TraceSeverity severity, const std::string &msg)
    {
        if( m_severity <= severity)
            std::cout<<msg<<std::endl;
    }

    void StdOutListener::Flush()
    {
        std::cout<<std::flush;
    }

    void StdOutListener::SetSeverity(TraceSeverity severity)
    {
        m_severity = severity;
    }
}
