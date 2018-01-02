#pragma once

#if defined(WIN32)
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif