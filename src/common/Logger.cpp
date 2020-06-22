#include "Logger.hpp"

#if defined(OS_WINDOWS)
#include <windows.h>
#endif

#if defined(OS_LINUX) || defined(OS_APPLE)
#include <csignal>
#endif

namespace OpenDemo
{
    namespace Common
    {
        void debugBreak()
        {
#if defined(OS_LINUX) || defined(OS_APPLE)
            raise(SIGTRAP);
#elif _MSC_VER && !__INTEL_COMPILER
            __debugbreak();
#else
            _asm { int 3 }
#endif
        }

        void Logger::Log(Level level, const U8String& msg)
        {
#if defined(OS_WINDOWS)
            OutputDebugStringW(StringConversions::UTF8ToWString(msg).c_str());
#else
            fmt::print(msg);
#endif
            
            if (level == Level::Fatal)
            {
                debugBreak();
                exit(1);
            }

        }
    }
}