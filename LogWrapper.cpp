#include "LogWrapper.h"


LogWrapper::LogWrapper()
{
}


LogWrapper::~LogWrapper()
{
}

void printLog(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}
