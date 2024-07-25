#pragma once

#include <algorithm>
#include <cmath>

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace RR
{
    static constexpr float EPS = std::numeric_limits<float>::epsilon();
    static constexpr float INF = std::numeric_limits<float>::infinity();
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float PI2 = PI * 2.0f;
    static constexpr float HALF_PI = PI / 2.0f;
    static constexpr float DEG2RAD = PI / 180.0f;
    static constexpr float RAD2DEG = 180.0f / PI;
    static constexpr float COS30 = 0.86602540378f;
    static constexpr float COS45 = 0.70710678118f;
    static constexpr float COS60 = 0.50000000000f;

    template <typename T>
    inline constexpr T Sqr(T x) { return x * x; };
    inline float FRandom() { return (float)((double)rand() / RAND_MAX); };

    using std::log;
    using std::log2;

    // Enum used for object construction specifying how the object should initialized.
    enum class Initialization
    {
        Identity
    };
    constexpr auto Identity = Initialization::Identity;

    // Angles
    enum class AngleUnitType : uint32_t
    {
        Radian,
        Degree
    };

    template <AngleUnitType UT, typename T>
    struct AngleUnitLimits;

    template <>
    struct AngleUnitLimits<AngleUnitType::Degree, float>
    {
        static inline float MAX = 360.0f;
        static inline float MIN = 0.0f;
    };

    template <>
    struct AngleUnitLimits<AngleUnitType::Radian, float>
    {
        static inline float MAX = PI2;
        static inline float MIN = 0.0f;
    };

    template <AngleUnitType UT>
    struct AngleUnit;

    using Radian = AngleUnit<AngleUnitType::Radian>;
    using Degree = AngleUnit<AngleUnitType::Degree>;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Common
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    inline void SinCos(float r, float* s, float* c)
    {
        // Todo use real SinCos
        *s = sinf(r);
        *c = cosf(r);
    }

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
    // AngleUnit
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    template <AngleUnitType UT>
    struct AngleUnit final
    {
        using FloatFormat = float;
        static_assert(std::is_floating_point<FloatFormat>::value, "Float point type assumed");
        static_assert(std::is_same<FloatFormat, float>::value, "Only float are supported now");

        constexpr AngleUnit() = default;
        constexpr AngleUnit(const AngleUnit<UT>& value) = default;
        constexpr AngleUnit& operator=(const AngleUnit<UT>& valued) = default;

        constexpr explicit AngleUnit(FloatFormat value) : value_(value) { }
        constexpr AngleUnit& operator=(const FloatFormat& value)
        {
            value_ = value;
            return *this;
        };

        template <AngleUnitType T, typename = std::enable_if_t<UT != T, void>>
        constexpr AngleUnit(AngleUnit<T> d) : value_(cast<T, UT>(d.Value())) { }

        template <AngleUnitType T, std::enable_if_t<UT != T, void>>
        constexpr AngleUnit(const AngleUnit<T>& d) : value_(cast<T, UT>(d.Value())) { }

        template <AngleUnitType T, std::enable_if_t<UT != T, void>>
        AngleUnit<UT>& operator=(const AngleUnit<T>& d)
        {
            value_ = cast<T, UT>(d.Value());
            return *this;
        }

        // todo degree.ToDegree() is workning?
        inline constexpr AngleUnit<AngleUnitType::Degree> ToDegree() const
        {
            return AngleUnit<AngleUnitType::Degree>(cast<UT, AngleUnitType::Degree>(value_));
        }

        inline constexpr AngleUnit<AngleUnitType::Radian> ToRadian() const
        {
            return AngleUnit<AngleUnitType::Radian>(cast<UT, AngleUnitType::Radian>(value_));
        }

        constexpr FloatFormat Value() const { return value_; }

        /** Wraps the angle */
        /*  AngleUnit<UT> Wrap()
        {
            T result = fmod(mDeg, AngleUnitLimits<UT, FloatFormat>::MAX);

            if (result < 0)
                result += static_cast<T>(AngleUnitLimits<UT, FloatFormat>::MAX);

            return AngleUnit<UT>(result);
        }*/

        const AngleUnit<UT>& operator+() const { return *this; }
        AngleUnit<UT> operator+(const AngleUnit<UT>& d) const { return AngleUnit<UT>(value_ + d.value_); }
        AngleUnit<UT> operator-() const { return AngleUnit<UT>(-value_); }
        AngleUnit<UT> operator-(const AngleUnit<UT>& d) const { return AngleUnit<UT>(value_ - d.value_); }
        AngleUnit<UT> operator*(FloatFormat f) const { return AngleUnit<UT>(value_ * f); }
        AngleUnit<UT> operator*(const AngleUnit<UT>& f) const { return AngleUnit<UT>(value_ * f.value_); }
        AngleUnit<UT> operator/(FloatFormat f) const { return AngleUnit<UT>(value_ / f); }

        AngleUnit<UT>& operator+=(const AngleUnit<UT>& d)
        {
            value_ += d.value_;
            return *this;
        }

        AngleUnit<UT>& operator-=(const AngleUnit<UT>& d)
        {
            value_ -= d.value_;
            return *this;
        }

        AngleUnit<UT>& operator+=(FloatFormat f)
        {
            value_ += f;
            return *this;
        }

        AngleUnit<UT>& operator-=(FloatFormat f)
        {
            value_ -= f;
            return *this;
        }

        AngleUnit<UT>& operator*=(FloatFormat f)
        {
            value_ *= f;
            return *this;
        }

        AngleUnit<UT>& operator/=(FloatFormat f)
        {
            value_ /= f;
            return *this;
        }

        template <AngleUnitType T, std::enable_if_t<UT != T>>
        AngleUnit<UT> operator+(const AngleUnit<T>& d) const { return AngleUnit<UT>(value_ + d.template cast<UT>()); }
        template <AngleUnitType T, std::enable_if_t<UT != T>>
        AngleUnit<UT> operator-(const AngleUnit<T>& d) const { return AngleUnit<UT>(value_ - d.template cast<UT>()); }

        template <AngleUnitType T, std::enable_if_t<UT != T>>
        AngleUnit<UT>& operator+=(const AngleUnit<T>& d)
        {
            value_ += d.template cast<UT>();
            return *this;
        }

        template <AngleUnitType T, typename std::enable_if<UT != T>::type>
        AngleUnit<UT>& operator-=(const AngleUnit<T>& d)
        {
            value_ -= d.template cast<UT>();
            return *this;
        }

        friend AngleUnit<UT> operator*(FloatFormat lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs * rhs.value_); }
        friend AngleUnit<UT> operator/(FloatFormat lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs / rhs.value_); }
        friend AngleUnit<UT> operator+(AngleUnit<UT>& lhs, FloatFormat rhs) { return AngleUnit<UT>(lhs.value_ + rhs); }
        friend AngleUnit<UT> operator+(FloatFormat lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs + rhs.value_); }
        friend AngleUnit<UT> operator-(const AngleUnit<UT>& lhs, FloatFormat rhs) { return AngleUnit<UT>(lhs.value_ - rhs); }
        friend AngleUnit<UT> operator-(const FloatFormat lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs - rhs.value_); }

        bool operator<(const AngleUnit<UT>& d) const { return value_ < d.value_; }
        bool operator<=(const AngleUnit<UT>& d) const { return value_ <= d.value_; }
        bool operator==(const AngleUnit<UT>& d) const { return value_ == d.value_; }
        bool operator!=(const AngleUnit<UT>& d) const { return value_ != d.value_; }
        bool operator>=(const AngleUnit<UT>& d) const { return value_ >= d.value_; }
        bool operator>(const AngleUnit<UT>& d) const { return value_ > d.value_; }

    private:
        template <AngleUnitType From, AngleUnitType To>
        static inline constexpr FloatFormat cast(FloatFormat value);

        template <>
        static inline constexpr FloatFormat cast<AngleUnitType::Degree, AngleUnitType::Radian>(FloatFormat value) { return value * DEG2RAD; }

        template <>
        static inline constexpr FloatFormat cast<AngleUnitType::Radian, AngleUnitType::Degree>(FloatFormat value) { return value * RAD2DEG; }

        FloatFormat value_ = 0.0f;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Acos & Asin
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    inline constexpr Radian Acos(float val)
    {
        if (-1.0f < val)
        {
            if (val < 1.0f)
                return Radian(std::acos(val));
            else
                return Radian(0.0f);
        }
        else
        {
            return Radian(PI);
        }
    }

    inline constexpr Radian Asin(float val)
    {
        if (-1.0f < val)
        {
            if (val < 1.0f)
                return Radian(std::asin(val));
            else
                return Radian(HALF_PI);
        }
        else
        {
            return Radian(-HALF_PI);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Align
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    inline constexpr T AlignTo(T value, size_t alignment)
    {
        static_assert(std::is_integral<T>::value, "Expect integral types.");
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
        static_assert(std::is_integral<T>::value, "Expect integral types.");
        return (value & (alignment - 1)) == 0;
    }

    template <typename T>
    inline constexpr bool IsAlignedTo(T* value, size_t alignment)
    {
        return (reinterpret_cast<uintptr_t>(value) & (alignment - 1)) == 0;
    }
}