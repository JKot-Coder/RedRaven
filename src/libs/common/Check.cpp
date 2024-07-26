#if !defined(DEBUG) && !defined(RELEASE) && !defined(RELEASE_WITH_DEBUG_INFO)
static_assert(false, "Build target undefined");
#endif

#if !defined(OS_WINDOWS) && !defined(OS_LINUX) && !defined(OS_APPLE)
static_assert(false, "Target OS undefined");
#endif