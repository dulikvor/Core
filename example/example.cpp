#include <string>
#include "src/Logger.h"
#include "src/Exception.h"
#include "src/Environment.h"
#include "src/GeneralParams.h"


int main( int argc, const char *argv[] )
{
    core::GeneralParams params;
    params.AddParam("str", "Hello world");
    std::string str = params.Get<std::string>("str");
    core::Logger::Instance().Start(core::TraceSeverity::Info);
    TRACE_INFO("Hello world");
    try
    {
        throw core::Exception(__CORE_SOURCE, "New exception");
    }
    catch(...)
    {
    }
    return 0;
}
