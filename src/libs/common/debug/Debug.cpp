#include "Debug.hpp"

#if OS_WINDOWS
#include <windows.h>
#endif

namespace OpenDemo
{
    namespace Common
    {
        namespace Debug
        {
            bool IsDebuggerPresent()
            {
#if OS_WINDOWS
                return ::IsDebuggerPresent();
#endif
            }
        }
    }
}