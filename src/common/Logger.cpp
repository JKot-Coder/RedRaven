#include "Logger.hpp"

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

        void Logger::Log(Level level, const std::string& msg)
        {
            fmt::print(msg);
            if (level == Level::Fatal)
                debugBreak();
        }
    }
}