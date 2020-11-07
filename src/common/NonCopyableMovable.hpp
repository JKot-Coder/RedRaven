#pragma once

namespace OpenDemo
{
    namespace Common
    {
        class NonCopyable
        {
        protected:
            NonCopyable() = default;
            ~NonCopyable() = default;

            NonCopyable(const NonCopyable&) = delete;
            NonCopyable& operator=(const NonCopyable&) = delete;
        };

        class NonMovable
        {
        protected:
            NonMovable() = default;
            ~NonMovable() = default;

            NonMovable(NonMovable&&) = delete;
            NonMovable& operator=(NonMovable&&) = delete;
        };
    }
}