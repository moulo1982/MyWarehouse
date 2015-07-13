#pragma once

#include <iostream>
#include <stdarg.h>
//#define LOG LogWrapper::WriteLog
#ifdef DEBUG
#define LOG printLog
#else
#define LOG
#endif
//#define LOG

void printLog(const char* format, ...);

class LogWrapper
{
public:
    LogWrapper();
    ~LogWrapper();

    template<typename... Args>
    static void WriteLog(Args... a)
    {
        DummtWrapper(print(a)...);
    }

private:
    template<typename T>
    static T print(T t)
    {
        std::cout << t;
        return t;
    }

    template<typename... Args>
    static void DummtWrapper(Args... a)
    {
    }
};

