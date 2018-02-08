#pragma once

#include <cstring>

namespace core {

    struct Source {
        const char *file;
        const char *function;
        int line;
    };
}

#define __CORE__FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __CORE_SOURCE core::Source{__CORE__FILENAME__, __FUNCTION__, __LINE__}

