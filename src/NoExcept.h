#pragma once

#if defined(WIN32)
#define NOEXCEPT(expression)
#else
#define NOEXCEPT(expression) noexcept(expression)
#endif