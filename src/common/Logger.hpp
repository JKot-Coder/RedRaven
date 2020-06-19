#pragma once

#include "common/String.hpp"

namespace OpenDemo
{

#ifdef DEBUG
#define ASSERT(exp) \
    if (!(exp))     \
        Log::Fatal(fmt::format("ASSERT:\n Value:{4} {0}:{1}\n {3} \n", __FILE__, __LINE__, __FUNCTION__, #exp));

#define ASSERT_MSG(exp, msg, ...) \
    if (!(exp))                   \
        Log::Fatal(fmt::format("ASSERT:\n Value:{4} {0}:{1}\n {3} => {5} \n", __FILE__, __LINE__, __FUNCTION__, #exp, fmt::format(msg, ##__VA_ARGS__)));
#else
#define ASSERT(ignore) ((void)0);
#endif

#define LOG_INFO(...) \
    Log::Info(fmt::format("INFO:\n  {0}:{1}\n  {2} => {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(##__VA_ARGS__)));

#define LOG_WARNING(...) \
    Log::Warning(fmt::format("WARNING:\n  {0}:{1}\n  {2} => {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(##__VA_ARGS__)));

#define LOG_ERROR(...) \
    Log::Error(fmt::format("EROR:\n  {0}:{1}\n  {2} => {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(##__VA_ARGS__)));

#define LOG_FATAL(...) \
    Log::Fatal(fmt::format("FATAL:\n  {0}:{1}\n  {2} => {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(##__VA_ARGS__)));

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

            static void Log(Level level, const U8String& msg);
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
