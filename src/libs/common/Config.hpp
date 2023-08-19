#pragma once

#if DEBUG && OS_WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#if DEBUG
#define ENABLE_ASSERTS 1
#endif

#if !DEBUG
#define ENABLE_INLINE 1
#endif

#if ENABLE_INLINE
#define INLINE inline
#else
#define INLINE
#endif // ENABLE_INLINE

#define ASAN_DEFAULT_OPTIONS             \
    __attribute__((used))                \
    __attribute__((no_sanitize_address)) \
                                         \
    extern "C" const char*               \
    __asan_default_options()             \
    {                                    \
        return "detect_leaks=1";         \
    }