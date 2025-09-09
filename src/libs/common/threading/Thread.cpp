#include "Thread.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#endif // OS_WINDOWS

#include "common/StringEncoding.hpp"

namespace RR::Common::Threading
{
    void Thread::SetName(std::string_view threadName)
    {
#ifdef OS_WINDOWS
        const auto& wThreadName = Common::StringEncoding::UTF8ToWide(threadName);
        SetThreadDescription(static_cast<HANDLE>(GetNativeHandle()), wThreadName.c_str());
#else
        ASSERT_MSG(false, "Not implemented");
#endif
    }
}