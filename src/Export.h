#pragma once

#if defined(WIN32)
    #ifdef A_DLL
    #define A_EXPORT __declspec(dllexport)
    #else
    #define A_EXPORT __declspec(dllimport)
    #endif
#else
    #define A_EXPORT
#endif