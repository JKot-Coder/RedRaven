#pragma once

#ifdef OS_WINDOWS
#include <windows.h>
#endif // OS_WINDOWS

namespace OpenDemo
{
    namespace Common
    {
#ifdef OS_WINDOWS
        using NativeWindowHandle = HWND;
#endif // OS_WINDOWS
    }
}