#pragma once

#include <csignal>

#ifdef DEBUG
    #if defined(OS_LINUX) || defined(OS_APPLE)
        #define debugBreak() raise(SIGTRAP);
    #else
        #define debugBreak() _asm { int 3 }
    #endif

    #define ASSERT(expr) if (expr) {} else { LOG("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, #expr); debugBreak(); }

    #ifndef _OS_ANDROID
        #define LOG(...) printf(__VA_ARGS__)
    #endif
#else
#define ASSERT(expr)
#ifdef OS_LINUX
#define LOG(...) printf(__VA_ARGS__); fflush(stdout)
#else
#define LOG(...) printf(__VA_ARGS__)
//    #define LOG(...) 0
#endif
#endif