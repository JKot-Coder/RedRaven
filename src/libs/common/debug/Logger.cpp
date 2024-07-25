#include "Logger.hpp"

#include "common/debug/DebugStream.hpp"
#include "common/StringEncoding.hpp"

#if defined(OS_WINDOWS)
#include <windows.h>
#endif

#if defined(OS_LINUX) || defined(OS_APPLE)
#include <csignal>
#endif

namespace RR
{
    struct InitConsole
    {
        InitConsole() { std::setlocale(LC_ALL, ""); }
    } initConsole;

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

            void Logger::Log(Level level, const std::string& msg)
            {
                std::ignore = level; // TODO

#if defined(OS_WINDOWS)
                // No utf-8 support;
                const auto& wstring = StringEncoding::UTF8ToWide(msg);
                Debug::WStream << wstring.c_str();
                std::wcerr << wstring.c_str();
#else
                Debug::Stream << msg.c_str();
#endif
            }
        }
    }
}