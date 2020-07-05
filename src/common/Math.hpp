#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <algorithm>

#define EPS FLT_EPSILON
#define INF INFINITY
#define PI 3.14159265358979323846f
#define PI2 (PI * 2.0f)
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#define COS30 0.86602540378f
#define COS45 0.70710678118f
#define COS60 0.50000000000f

#define SQR(x) ((x) * (x))
#define randf() ((float)rand() / RAND_MAX)

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace OpenDemo
{
    namespace Common
    {
        using Vector2 = Eigen::Vector2f;
        using Vector3 = Eigen::Vector3f;
        using Vector4 = Eigen::Vector4f;

        using Vector2i = Eigen::Vector2i;
        using Vector3i = Eigen::Vector3i;
        using Vector4i = Eigen::Vector4i;

        using Matrix4 = Eigen::Matrix4f;
        using Quaternion = Eigen::Quaternionf;

        using AlignedBox2i = Eigen::AlignedBox2i;

        using Affine3 = Eigen::Affine3f;

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

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // PointerMath
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

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