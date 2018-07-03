#pragma once

#if defined(WIN32)
    #ifdef CORE_DLL
    #define CORE_EXPORT __declspec(dllexport)
    #else
    #define CORE_EXPORT __declspec(dllimport)
    #endif
#else
    #define CORE_EXPORT
#endif