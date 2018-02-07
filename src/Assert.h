#pragma once

#include <errno.h>
#include <stdlib.h>
#include "Exception.h"
#define PLATFORM_VERIFY(expression) do{ if((expression) == false) throw core::Exception(SOURCE, "An error occured, Reason - %s", strerror(errno)); } while(0)
#define VERIFY(expression, ...) do{ if((expression) == false) throw core::Exception(SOURCE, ##__VA_ARGS__ ); } while(0)
