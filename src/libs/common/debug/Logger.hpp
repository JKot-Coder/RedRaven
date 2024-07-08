#pragma once

#include <cassert>
#include <tuple>

#include "common/String.hpp"

namespace RR
{
    namespace Common
    {
        namespace Debug
        {
#ifdef ENABLE_ASSERTS
#ifdef MSVVVvVV // TODO
#define ASSERT(exp, ...)                                                                                                                        \
    static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "ASSERT takes only one argument use ASSET_MSG instead"); \
    if (!(exp))                                                                                                                                 \
        Log::Format::Fatal("ASSERT: {3}\n  {0}({1}): {2}\n", __FILE__, __LINE__, __FUNCTION__, #exp)
#else
#define ASSERT(exp)                                                                                   \
    if (!(exp))                                                                                       \
    {                                                                                                 \
        Log::Format::Fatal("ASSERT: {3}\n  {0}({1}): {2}\n", __FILE__, __LINE__, __FUNCTION__, #exp); \
        assert(0);                                                                                    \
    } (void)0
#endif
#define ASSERT_MSG(exp, ...)                                                                                                           \
    if (!(exp))                                                                                                                        \
    {                                                                                                                                  \
        Log::Format::Fatal("ASSERT: {3}\n  {0}({1}): {2}\n  {4}\n", __FILE__, __LINE__, __FUNCTION__, #exp, fmt::format(__VA_ARGS__)); \
        assert(0);                                                                                                                     \
    } (void)0
#else
#define ASSERT(ignore) ((void)0)
#define ASSERT_MSG(ignore, ...) ((void)0)
#endif

#define LOG_INFO(...) \
    Log::Format::Info("INFO:\n  {0}({1}): {2}\n  {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define LOG_WARNING(...) \
    Log::Format::Warning("WARNING:\n  {0}({1}): {2}\n  {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define LOG_ERROR(...) \
    Log::Format::Error("ERROR:\n  {0}({1}): {2}\n  {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

#define LOG_FATAL(...) \
    Log::Format::Fatal("FATAL:\n  {0}({1}): {2}\n  {3}\n", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__))

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

            namespace Log
            {
                namespace Print
                {
                    template <typename S, typename... Args>
                    inline void Info(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Info, fmt::sprintf(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Warning(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Warning, fmt::sprintf(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Error(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Error, fmt::sprintf(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Fatal(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Fatal, fmt::sprintf(format, args...));
                    }
                }
                namespace Format
                {
                    template <typename S, typename... Args>
                    inline void Info(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Info, fmt::format(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Warning(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Warning, fmt::format(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Error(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Error, fmt::format(format, args...));
                    }

                    template <typename S, typename... Args>
                    inline void Fatal(const S& format, Args&&... args)
                    {
                        Logger::Log(Logger::Level::Fatal, fmt::format(format, args...));
                    }
                }
            }
        }
    }
}