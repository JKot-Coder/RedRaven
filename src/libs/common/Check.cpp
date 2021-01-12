#if !defined(DEBUG) && !defined(RELEASE)
static_assert(false, "Build target undefined");
#endif

#if !defined(OS_WINDOWS) && !defined(OS_LINUX) && !defined(OS_APPLE)
static_assert(false, "Target OS undefined");
#endif