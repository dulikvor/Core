#pragma once

#include <errno.h>
#include <stdlib.h>
#include "Exception.h"

#define LINUX_VERIFY(expression) do{ if(expression == false) throw core::Exception(SOURCE, "An error occured, Reason - %s", strerror(errno)); } while(0)
#define VERIFY(expression, format, ...) do{ if(expression == false) throw core::Exception(SOURCE, format, ##__VA_ARGS__ ); } while(0)
#define ASSERT(expression) do{ if(expression == false) abort(); } while(0)
