#include "Logger.hpp"

#include "common/debug/DebugStream.hpp"

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
        namespace Debug
        {
            void debugBreak()
            {
#if defined(OS_LINUX) || defined(OS_APPLE)
                raise(SIGTRAP);
#elif _MSC_VER && !__INTEL_COMPILER
                __debugbreak();
#else
                __asm__("int3");
#endif
            }

            void Logger::Log(Level level, const U8String& msg)
            {
#if defined(OS_WINDOWS)
                // No utf-8 support;
                Debug::WStream << StringConversions::UTF8ToWString(msg).c_str();
                std::wcerr << StringConversions::UTF8ToWString(msg).c_str();
#else
                Debug::Stream << msg.c_str();
#endif

                if (level == Level::Fatal)
                {
                    //_CrtDbgReportW;
                    debugBreak();
                    exit(1);
                }
            }
        }
    }
}