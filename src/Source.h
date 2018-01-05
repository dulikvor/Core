#pragma once

#include <cstring>

struct Source
{
    const char* file;
    const char* function;
    int line;
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define SOURCE Source{__FILENAME__, __FUNCTION__, __LINE__}

