#pragma once

#include <errno.h>
#include <stdlib.h>
#include "Exception.h"
#define PLATFORM_VERIFY(expression) do{ if((expression) == false) throw core::Exception(__CORE_SOURCE, "An error occured, Reason - %s, error code - %d", strerror(errno), errno); } while(0)
#define VERIFY(expression, ...) do{ if((expression) == false) throw core::Exception(__CORE_SOURCE, __VA_ARGS__ ); } while(0)
