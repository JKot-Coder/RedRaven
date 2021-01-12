#pragma once

#ifdef DEBUG 
#define ENABLE_ASSERTS
#endif

#ifndef DEBUG
#define ENABLE_INLINE
#endif

#ifdef ENABLE_INLINE
#define INLINE inline
#else
#define INLINE
#endif // ENABLE_INLINE
