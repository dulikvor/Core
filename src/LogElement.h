#pragma once

#include <time.h>
#include <string>
/*LogElement is a static struct which every single instance of it represent a request for logging, the
struct contains the following states:
1) line and the source file name which the logging request originates from.
2) The time upon which the logging was requrested.
3) The message it self.*/
struct LogElement
{
    LogElement(int newLine, const char* newFile, const std::string& newMessage,
               const time_t& newTraceTime) : line(newLine), file(newFile), message(newMessage),
                traceTime(newTraceTime){}

    const int line;
    const std::string file;
    const std::string message;
    const time_t traceTime;
};
