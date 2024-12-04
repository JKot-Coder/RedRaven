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

#define UNUSED(...) (void)(sizeof(__VA_ARGS__))

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__clang__)
#define LIKELY(x) (__builtin_expect(!!(x), 1))
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#elif defined(__cpp_attributes) && __cpp_attributes >= 201803L
// For compilers supporting C++20 [[likely]]
#define LIKELY(x) (x) [[likely]]
#define UNLIKELY(x) (x) [[unlikely]]
#else
#define LIKELY(x)(x)
#define UNLIKELY(x) (x)
#endif

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