#pragma once

#if DEBUG && OS_WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#if DEBUG || FORCE_ENABLE_ASSERTS
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