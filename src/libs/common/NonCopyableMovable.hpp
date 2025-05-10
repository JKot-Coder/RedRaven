#pragma once

namespace RR::Common
{
    class NonCopyable
    {
    protected:
        NonCopyable() = default;
        ~NonCopyable() = default;
    public:
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = default;
        NonCopyable& operator=(NonCopyable&&) = default;
    };

    class NonMovable
    {
    protected:
        NonMovable() = default;
        ~NonMovable() = default;
    public:
        NonMovable(const NonMovable&) = default;
        NonMovable& operator=(const NonMovable&) = default;
        NonMovable(NonMovable&&) = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };

    class NonCopyableMovable : public NonCopyable, public NonMovable
    {
    };
}