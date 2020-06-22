#pragma once

#include <tuple>

#include "common/String.hpp"

namespace OpenDemo
{

#ifdef ENABLE_ASSERTS
#define ASSERT(exp, ...)                                                                                                                        \
    static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "ASSERT takes only one argument use ASSET_MSG instead"); \
    if (!(exp))                                                                                                                                 \
        Log::Print::Fatal(fmt::format("ASSERT: {3}\n  {0}({1}): {3}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, #exp));

#define ASSERT_MSG(exp, ...) \
    if (!(exp))              \
        Log::Print::Fatal(fmt::format("ASSERT: {3}\n  {0}({1}): {4}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, #exp, fmt::sprintf(__VA_ARGS__)));
#else
#define ASSERT(ignore) ((void)0);
#endif

#define LOG_INFO(...) \
    Log::Print::Info(fmt::format("INFO:\n  {0}({1}): {3}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, fmt::sprintf(__VA_ARGS__)));

#define LOG_WARNING(...) \
    Log::Print::Warning(fmt::format("WARNING:\n  {0}({1}): {3}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, fmt::sprintf(__VA_ARGS__)));

#define LOG_ERROR(...) \
    Log::Print::Error(fmt::format("ERROR:\n  {0}({1}): {3}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, fmt::sprintf(__VA_ARGS__)));

#define LOG_FATAL(...) \
    Log::Print::Fatal(fmt::format("FATAL:\n  {0}({1}): {3}\n  {2}\n", __FILE__, __LINE__, __FUNCTION__, fmt::sprintf(__VA_ARGS__)));

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
        namespace Print
        {

            template <typename S, typename... Args>
            inline void Info(const S& format, Args&&... args)
            {
                Common::Logger::Log(Common::Logger::Level::Info, fmt::sprintf(format, args...));
            }

            template <typename S, typename... Args>
            inline void Warning(const S& format, Args&&... args)
            {
                Common::Logger::Log(Common::Logger::Level::Warning, fmt::sprintf(format, args...));
            }

            template <typename S, typename... Args>
            inline void Error(const S& format, Args&&... args)
            {
                Common::Logger::Log(Common::Logger::Level::Error, fmt::sprintf(format, args...));
            }

            template <typename S, typename... Args>
            inline void Fatal(const S& format, Args&&... args)
            {
                Common::Logger::Log(Common::Logger::Level::Fatal, fmt::sprintf(format, args...));
            }
        }
        namespace Format
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
}
