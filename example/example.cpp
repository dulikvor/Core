#include <string>
#include "src/Logger.h"


int main( int argc, const char *argv[] )
{
    core::Logger::Instance().Start(core::TraceSeverity::Info);
    TRACE_INFO("Hello world");
    return 0;
}
