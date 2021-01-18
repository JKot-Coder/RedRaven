#pragma once

#if DEBUG && OS_WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#ifdef DEBUG 
#define ENABLE_ASSERTS 1
#endif

#ifndef DEBUG
#define ENABLE_INLINE 1
#endif

#ifdef ENABLE_INLINE
#define INLINE inline
#else
#define INLINE
#endif // ENABLE_INLINE
