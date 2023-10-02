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

#define UNUSED(x) ((void)(x))

#ifdef _MSC_VER
#define USED
#define NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#else
#define USED __attribute__((used))
#define NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#endif

// clang-format off
#define ASAN_DEFAULT_OPTIONS     \
    USED                         \
    NO_SANITIZE_ADDRESS          \
                                 \
    extern "C" const char*       \
    __asan_default_options()     \
    {                            \
        return "detect_leaks=1"; \
    }
// clang-format on