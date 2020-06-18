#pragma once

#include "dependencies/fmt/include/fmt/format.h"

namespace OpenDemo
{

#ifdef DEBUG
#define ASSERT(expr) \
    if (!expr)       \
        Log::Fatal(fmt::format("ASSERT:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(#expr)));
#else
#define ASSERT(expr)
#endif

#define LOG_INFO(expr) \
    Log::Info(fmt::format("INFO:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(#expr)))

#define LOG_WARNING(expr) \
    Log::Warning(fmt::format("WARNING:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(#expr)))

#define LOG_ERROR(expr) \
    Log::Error(fmt::format("EROR:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(#expr)))

#define LOG_FATAL(expr) \
    Log::Fatal(fmt::format("FATAL:\n  %s:%d\n  %s => %s\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(#expr)))

    namespace Common
    {
        class Logger
        {
        public:
            enum class Level
            {
                Info = 0, /// Informative messages.
                Warning = 1, /// Warning messages.
                Error = 2, /// Error messages. Application might be able to continue running, but incorrectly.
                Fatal = 3, /// Unrecoverable error messages. Terminates application immediately.
                Disabled = -1
            };

            static void Log(Level level, const std::string& msg);
        };
    }

    namespace Log
    {
        template <typename S, typename... Args>
        inline void Info(const S& format, Args&&... args)
        {
            Common::Logger::Log(Common::Logger::Level::Info, fmt::format(format, args...));
        }

        template <typename S, typename... Args>
        inline void Warning(const S& format, Args&&... args)
        {
            Common::Logger::Log(Common::Logger::Level::Warning, fmt::format(format, args...));
        }

        template <typename S, typename... Args>
        inline void Error(const S& format, Args&&... args)
        {
            Common::Logger::Log(Common::Logger::Level::Error, fmt::format(format, args...));
        }

        template <typename S, typename... Args>
        inline void Fatal(const S& format, Args&&... args)
        {
            Common::Logger::Log(Common::Logger::Level::Fatal, fmt::format(format, args...));
        }
    }
}
