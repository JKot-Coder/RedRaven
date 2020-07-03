#pragma once

#include <algorithm>

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace OpenDemo
{
    namespace Common
    {
        // TODO CAPS
        template <typename T>
        inline constexpr T min(const T a, const T b)
        {
            return a < b ? a : b;
        }

        template <typename T>
        inline constexpr T min(const T a, const T b, const T c)
        {
            return (a < b && a < c) ? a : ((b < a && b < c) ? b : c);
        }

        template <class T>
        inline constexpr T max(const T a, const T b)
        {
            return a > b ? a : b;
        }

        template <typename T>
        inline constexpr T max(const T a, const T b, const T c)
        {
            return (a > b && a > c) ? a : ((b > a && b > c) ? b : c);
        }

        template <class T>
        inline constexpr T clamp(const T x, const T a, const T b)
        {
            return x < a ? a : (x > b ? b : x);
        }

        template <typename T>
        inline constexpr int sign(const T val)
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
            ASSERT(value < MaxPowerOfTwo<T>())
#else
            // TODO add warning?
            value = min(value, MaxPowerOfTwo<T>())
#endif

            --value;
            for (size_t i = 1; i != CHAR_BIT * sizeof(T); i <<= 1)
            {
                value |= value >> i;
            }
            ++value;
            return value;
        }

        template <typename T>
        struct Vector2
        {
            Vector2() = default;

            Vector2(T s)
                : x(s)
                , y(s)
            {
            }

            Vector2<T>(T x, T y)
                : x(x)
                , y(y)
            {
            }

            template <typename U>
            Vector2<T>::Vector2(const Vector2<U>& vector)
                : x(static_cast<T>(vector.x))
                , y(static_cast<T>(vector.y))
            {
            }

            T& operator[](int index) const;

            bool operator==(const Vector2<T>& v) const;
            bool operator!=(const Vector2<T>& v) const;
            bool operator==(T s) const;
            bool operator!=(T s) const;
            bool operator<(const Vector2<T>& v) const;
            bool operator>(const Vector2<T>& v) const;

            Vector2<T> operator-() const;
            Vector2<T>& operator+=(const Vector2<T>& v);
            Vector2<T>& operator-=(const Vector2<T>& v);
            Vector2<T>& operator*=(const Vector2<T>& v);
            Vector2<T>& operator/=(const Vector2<T>& v);
            Vector2<T>& operator+=(T s);
            Vector2<T>& operator-=(T s);
            Vector2<T>& operator*=(T s);
            Vector2<T>& operator/=(T s);
            Vector2<T> operator+(const Vector2<T>& v) const;
            Vector2<T> operator-(const Vector2<T>& v) const;
            Vector2<T> operator*(const Vector2<T>& v) const;
            Vector2<T> operator/(const Vector2<T>& v) const;
            Vector2<T> operator+(T s) const;
            Vector2<T> operator-(T s) const;
            Vector2<T> operator*(T s) const;
            Vector2<T> operator/(T s) const;

            T dot(const Vector2<T>& v) const;
            T cross(const Vector2<T>& v) const;

            T length2() const;
            T length() const;

            Vector2<T> abs() const;
            Vector2<T> normal() const;
            T angle() const;

            Vector2<T>& rotate(const Vector2<T>& cs);
            Vector2<T>& rotate(T angle);

            T x;
            T y;
        };

        template <typename T>
        struct Rect
        {
            Rect() = default;
            Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight)
                : left(rectLeft)
                , top(rectTop)
                , width(rectWidth)
                , height(rectHeight) {};

            Rect(const Vector2<T>& position, const Vector2<T>& size)
                : left(position.x)
                , top(position.y)
                , width(size.x)
                , height(size.y)
            {
            }

            template <typename U>
            explicit Rect(const Rect<U>& rectangle)
                : left(static_cast<T>(rectangle.left))
                , top(static_cast<T>(rectangle.top))
                , width(static_cast<T>(rectangle.width))
                , height(static_cast<T>(rectangle.height)) {};

            bool contains(T x, T y) const;
            bool contains(const Vector2<T>& point) const;
            bool intersects(const Rect<T>& rect) const;
            bool intersects(const Rect<T>& rect, Rect<T>& intersection) const;

            Vector2<T> getPosition() const;
            Vector2<T> getSize() const;

            bool operator==(const Rect<T>& rect) const;
            bool operator!=(const Rect<T>& rect) const;

            T left;
            T top;
            T width;
            T height;
        };

        using Vector2F = Vector2<float>;
        using Vector2U32 = Vector2<uint32_t>;
        using Vector2U = Vector2U32;

        using RectU32 = Rect<uint32_t>;
        using RectU = RectU32;

        namespace PointerMath
        {
            template <typename T>
            inline constexpr T AlignTo(T value, size_t alignment)
            {
                static_assert(std::is_integral<T>::value, "Expect integral types.");
                ASSERT(IsPowerOfTwo(alignment))
                const size_t bumpedValue = static_cast<size_t>(value) + (alignment - 1);
                const size_t truncatedValue = bumpedValue & ~(alignment - 1);
                return static_cast<T>(truncatedValue);
            }

            template <typename PtrType>
            inline constexpr PtrType* AlignTo(PtrType* value, size_t alignment)
            {
                return reinterpret_cast<PtrType*>(AlignTo(reinterpret_cast<uintptr_t>(value), alignment));
            }

            template <typename T>
            inline constexpr bool IsAlignedTo(T value, size_t alignment)
            {
                static_assert(std::is_integral<T>::value, "Expect integral types.");
                return (value & (alignment - 1)) == 0;
            }

            template <typename PtrType>
            inline constexpr bool IsAlignedTo(PtrType* value, size_t alignment)
            {
                return (reinterpret_cast<uintptr_t>(value) & (alignment - 1)) == 0;
            }
        }
    }
}

#ifdef ENABLE_INLINE
#include <Math.inl>
#endif