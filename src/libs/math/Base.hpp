#pragma once

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace RR
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Common
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    inline constexpr T Min(const T a, const T b)
    {
        return a < b ? a : b;
    }

    template <typename T>
    inline constexpr T Min(const T a, const T b, const T c)
    {
        return (a < b && a < c) ? a : ((b < a && b < c) ? b : c);
    }

    template <class T>
    inline constexpr T Max(const T a, const T b)
    {
        return a > b ? a : b;
    }

    template <typename T>
    inline constexpr T Max(const T a, const T b, const T c)
    {
        return (a > b && a > c) ? a : ((b > a && b > c) ? b : c);
    }

    template <class T>
    inline constexpr T Clamp(const T x, const T a, const T b)
    {
        return x < a ? a : (x > b ? b : x);
    }

    template <typename T>
    inline constexpr int Sign(const T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    template <typename T>
    inline constexpr bool IsPowerOfTwo(const T value)
    {
        static_assert(std::is_integral<T>::value, "Expect integral types.");
        return value != 0 && (value & (value - 1)) == 0;
    }

    template <typename T>
    inline constexpr T MaxPowerOfTwo()
    {
        static_assert(std::is_integral<T>::value, "Expect integral types.");
        return (std::numeric_limits<T>::max() >> 1) + 1;
    }

    template <typename T>
    inline constexpr T RoundUpToPowerOfTwo(T value)
    {
        static_assert(std::is_integral<T>::value, "Expect integral types.");

        if (value == 0)
            return 1;

#ifdef ENABLE_ASSERTS
        ASSERT(value < MaxPowerOfTwo<T>());
#else
        // TODO add warning?
        value = std::min(value, MaxPowerOfTwo<T>());
#endif

        --value;
        for (size_t i = 1; i != CHAR_BIT * sizeof(T); i <<= 1)
        {
            value |= value >> i;
        }
        ++value;
        return value;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Align
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    inline constexpr T AlignTo(T value, size_t alignment)
    {
        static_assert(std::is_integral_v<T>, "Expect integral types.");
        ASSERT(IsPowerOfTwo(alignment));
        const size_t bumpedValue = static_cast<size_t>(value) + (alignment - 1);
        const size_t truncatedValue = bumpedValue & ~(alignment - 1);
        return static_cast<T>(truncatedValue);
    }

    template <typename T>
    inline constexpr T* AlignTo(T* value, size_t alignment)
    {
        return reinterpret_cast<T*>(AlignTo(reinterpret_cast<uintptr_t>(value), alignment));
    }

    template <typename T>
    inline constexpr bool IsAlignedTo(T value, size_t alignment)
    {
        static_assert(std::is_integral_v<T>, "Expect integral types.");
        return (value & (alignment - 1)) == 0;
    }

    template <typename T>
    inline constexpr bool IsAlignedTo(T* value, size_t alignment)
    {
        return (reinterpret_cast<uintptr_t>(value) & (alignment - 1)) == 0;
    }
}