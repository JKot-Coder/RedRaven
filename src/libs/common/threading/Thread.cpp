#include "Thread.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#elif defined(OS_APPLE) || defined(OS_LINUX)
#include <pthread.h>
#include <cstring>
#endif

#include "common/StringEncoding.hpp"

namespace RR::Common::Threading
{
    void Thread::setName(const std::string& threadName)
    {
#ifdef OS_WINDOWS
        const auto& wThreadName = Common::StringEncoding::UTF8ToWide(threadName);
        SetThreadDescription(static_cast<HANDLE>(GetNativeHandle()), wThreadName.c_str());
#elif defined(OS_APPLE)
        pthread_setname_np(threadName.c_str());
#else
        UNUSED(threadName);
        ASSERT_MSG(false, "Not implemented");
#endif
    }
}