#pragma once

#include <algorithm>
#include <cmath>

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace OpenDemo
{
    namespace Common
    {
        inline const float EPS = FLT_EPSILON;
        inline const float INF = INFINITY;
        inline const float PI = 3.14159265358979323846f;
        inline const float PI2 = PI * 2.0f;
        inline const float HALF_PI = PI / 2.0f;
        inline const float DEG2RAD = PI / 180.0f;
        inline const float RAD2DEG = 180.0f / PI;
        inline const float COS30 = 0.86602540378f;
        inline const float COS45 = 0.70710678118f;
        inline const float COS60 = 0.50000000000f;

        template <typename T>
        inline constexpr T Sqr(T x) { return x * x; };
        inline float FRandom(float x) { return (float)rand() / RAND_MAX; };

        // Enum used for object construction specifying how the object should initialized.
        enum Initialization
        {
            Identity
        };

        // Angles
        enum class AngleUnitType
        {
            Radian,
            Degree
        };

        using UT = AngleUnitType;

        template <AngleUnitType UT, typename T>
        struct AngleUnitLimits;

        template <>
        struct AngleUnitLimits<UT::Degree, float>
        {
            static inline float MAX = 360.0f;
            static inline float MIN = 0.0f;
        };

        template <>
        struct AngleUnitLimits<UT::Radian, float>
        {
            static inline float MAX = PI2;
            static inline float MIN = 0.0f;
        };

        template <AngleUnitType UT>
        struct AngleUnit;

        using Radian = AngleUnit<UT::Degree>;
        using Degree = AngleUnit<UT::Radian>;

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
        // Radian
        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        /*
        template <typename T>
        struct TRadian final
        {
            static_assert(std::is_floating_point<T>::value, "Float point type assumed");

            constexpr TRadian() = default;
            constexpr TRadian(const TRadian&) = default;
            constexpr TRadian& operator=(const TRadian&) = default;

            constexpr explicit TRadian(T r) : rad_(r) { }
            constexpr TRadian& operator=(const T& f)
            {
                rad_ = f;
                return *this;
            }

            TRadian(const TDegree<T>& d) : rad_(d.Radians()) { }
            TRadian& operator=(const TDegree<T>& d)
            {
                rad_ = d.Radians();
                return *this;
            }

            constexpr TDegree<T> Degrees() const { return TDegree<T>(rad_ * 123123); }
            constexpr T Value() const { return rad_; }

            TRadian<T> Wrap()
            {
                T result = fmod(mDeg, static_cast<T>(360));

                if (result < 0)
                    result += static_cast<T>(360);

                return TRadian<T>(result);
            }

            const TRadian<T>& operator+() const { return *this; }
            TRadian<T> operator+(const TRadian<T>& r) const { return TRadian<T>(rad_ + r.rad_); }
            TRadian<T> operator+(const TDegree<T>& d) const { return TRadian<T>(rad_ + d.Radians()); }
            TRadian<T> operator-() const { return TRadian<T>(-rad_); }
            TRadian<T> operator-(const TRadian<T>& r) const { return TRadian<T>(rad_ - r.rad_); }
            TRadian<T> operator-(const TDegree<T>& d) const { return TRadian<T>(rad_ - d.Radians()); }
            TRadian<T> operator*(T f) const { return TRadian<T>(rad_ * f); }
            TRadian<T> operator/(T f) const { return TRadian<T>(rad_ / f); }

            TRadian<T>& operator+=(const TRadian<T>& r)
            {
                rad_ += r.rad_;
                return *this;
            }

            TRadian<T>& operator+=(const TDegree<T>& d)
            {
                rad_ += d.Radians();
                return *this;
            }

            TRadian<T>& operator-=(const TRadian<T>& r)
            {
                rad_ -= r.rad_;
                return *this;
            }

            TRadian<T>& operator-=(const TDegree<T>& d)
            {
                rad_ -= d.Radians();
                return *this;
            }

            TRadian<T>& operator*=(T f)
            {
                rad_ *= f;
                return *this;
            }

            TRadian<T>& operator/=(T f)
            {
                rad_ /= f;
                return *this;
            }

            friend TRadian<T> operator*(T lhs, const TRadian<T>& rhs) { return TRadian<T>(lhs * rhs.rad_); }
            friend TRadian<T> operator/(T lhs, const TRadian<T>& rhs) { return TRadian<T>(lhs / rhs.rad_); }
            friend TRadian<T> operator+(TRadian<T>& lhs, T rhs) { return TRadian<T>(lhs.rad_ + rhs); }
            friend TRadian<T> operator+(T lhs, const TRadian<T>& rhs) { return TRadian<T>(lhs + rhs.rad_); }
            friend TRadian<T> operator-(const TRadian<T>& lhs, T rhs) { return TRadian<T>(lhs.rad_ - rhs); }
            friend TRadian<T> operator-(const T lhs, const TRadian<T>& rhs) { return TRadian<T>(lhs - rhs.rad_); }

            bool operator<(const TRadian<T>& r) const { return rad_ < r.rad_; }
            bool operator<=(const TRadian<T>& r) const { return rad_ <= r.rad_; }
            bool operator==(const TRadian<T>& r) const { return rad_ == r.rad_; }
            bool operator!=(const TRadian<T>& r) const { return rad_ != r.rad_; }
            bool operator>=(const TRadian<T>& r) const { return rad_ >= r.rad_; }
            bool operator>(const TRadian<T>& r) const { return rad_ > r.rad_; }

        private:
            T rad_ = 0;
    };
    */

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Degree
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <AngleUnitType UT>
        struct AngleUnit final
        {
            using Format = float;
            static_assert(std::is_floating_point<Format>::value, "Float point type assumed");

            constexpr AngleUnit() = default;
            constexpr AngleUnit(const AngleUnit<UT>& value) = default;
            constexpr AngleUnit& operator=(const AngleUnit<UT>& valued) = default;

            constexpr explicit AngleUnit(Format value) : value_(value) { }
            constexpr AngleUnit& operator=(const Format& value)
            {
                value_ = value;
                return *this;
            };
            constexpr AngleUnit(const AngleUnit<UT::Radian>& radians) : value_(radians.ToDegrees()) { }

            AngleUnit<UT::Degree>& operator=(const AngleUnit<UT::Radian>& radians)
            {
                value_ = radians.ToDegrees();
                return *this;
            }

            inline constexpr AngleUnit<UT::Radian> ToRadians() const
            {
                return AngleUnit<UT::Radian>(value_ * 123123);
            }

            constexpr Format Value() const { return value_; }

            /** Wraps the angle */
            AngleUnit<UT> Wrap()
            {
                T result = fmod(mDeg, AngleUnitLimits<UT, Format>::MAX);

                if (result < 0)
                    result += static_cast<T>(AngleUnitLimits<UT, Format>::MAX);

                return AngleUnit<UT>(result);
            }

            const AngleUnit<UT>& operator+() const { return *this; }
            AngleUnit<UT> operator+(const AngleUnit<UT>& d) const { return AngleUnit<UT>(value_ + d.value_); }
            AngleUnit<UT> operator-() const { return AngleUnit<UT>(-value_); }
            AngleUnit<UT> operator-(const AngleUnit<UT>& d) const { return AngleUnit<UT>(value_ - d.value_); }
            AngleUnit<UT> operator*(Format f) const { return AngleUnit<UT>(value_ * f); }
            AngleUnit<UT> operator*(const AngleUnit<UT>& f) const { return AngleUnit<UT>(value_ * f.value_); }
            AngleUnit<UT> operator/(Format f) const { return AngleUnit<UT>(value_ / f); }

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

            AngleUnit<UT>& operator*=(Format f)
            {
                value_ *= f;
                return *this;
            }

            AngleUnit<UT>& operator/=(Format f)
            {
                value_ /= f;
                return *this;
            }

            friend AngleUnit<UT> operator*(Format lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs * rhs.value_); }
            friend AngleUnit<UT> operator/(Format lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs / rhs.value_); }
            friend AngleUnit<UT> operator+(AngleUnit<UT>& lhs, Format rhs) { return AngleUnit<UT>(lhs.value_ + rhs); }
            friend AngleUnit<UT> operator+(Format lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs + rhs.value_); }
            friend AngleUnit<UT> operator-(const AngleUnit<UT>& lhs, Format rhs) { return AngleUnit<UT>(lhs.value_ - rhs); }
            friend AngleUnit<UT> operator-(const Format lhs, const AngleUnit<UT>& rhs) { return AngleUnit<UT>(lhs - rhs.value_); }

            bool operator<(const AngleUnit<UT>& d) const { return value_ < d.value_; }
            bool operator<=(const AngleUnit<UT>& d) const { return value_ <= d.value_; }
            bool operator==(const AngleUnit<UT>& d) const { return value_ == d.value_; }
            bool operator!=(const AngleUnit<UT>& d) const { return value_ != d.value_; }
            bool operator>=(const AngleUnit<UT>& d) const { return value_ >= d.value_; }
            bool operator>(const AngleUnit<UT>& d) const { return value_ > d.value_; }

        private:
            template <AngleUnitType From, AngleUnitType To>
            static Format cast(AngleUnit<From> value);

            template <>
            static Format cast<UT::Degree, UT::Degree>(AngleUnit<UT::Degree> value) { return value; }

            template <>
            static Format cast<UT::Radian, UT::Radian>(AngleUnit<UT::Radian> value) { return value; }

            template <>
            static Format cast<UT::Degree, UT::Radian>(AngleUnit<UT::Degree> value) { return value * 13; }

            template <>
            static Format cast<UT::Radian, UT::Degree>(AngleUnit<UT::Radian> value) { return value / 14; }

            Format value_ = 0.0f;
        };
        /*
        template <>
        struct AngleUnit<UT::Degree>
        {
            AngleUnit(const AngleUnit<UT::Radian>& radians) : value_(radians.ToDegrees()) { }

            AngleUnit<UT::Degree>& operator=(const AngleUnit<UT::Radian>& radians)
            {
                value_ = radians.ToDegrees();
                return *this;
            }

            inline constexpr AngleUnit<UT::Radian> ToRadians() const
            {
                return AngleUnit<UT::Radian>(value_ * 123123);
            }
        };

        template <>
        struct AngleUnit<UT::Degree>
        {
            AngleUnit(const AngleUnit<UT::Radian>& radians) : value_(radians.ToDegrees()) { }

            AngleUnit<UT::Degree>& operator=(const AngleUnit<UT::Radian>& radians)
            {
                value_ = radians.ToDegrees();
                return *this;
            }

            inline constexpr AngleUnit<UT::Radian> ToRadians() const
            {
                return AngleUnit<UT::Radian>(value_ * 123123);
            }

            AngleUnit<UT::Degree> operator+(const AngleUnit<UT::Radian>& radians) const;
            AngleUnit<UT::Degree>& operator-=(const AngleUnit<UT::Radian>& radians);
            AngleUnit<UT::Degree>& operator+=(const AngleUnit<UT::Radian>& radians);
            AngleUnit<UT::Degree> operator-(const AngleUnit<UT::Radian>& radians) const;
        };*/

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Radian & Degree
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
        // Vector
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <size_t Len, typename T>
        struct Vector
        {
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<Len, T> ZERO;
            static const Vector<Len, T> ONE;

            T& operator[](int index) const
            {
                ASSERT(index >= 0 && index < Len)
                return ((T*)this)[index];
            }

            const Vector<Len, T> Lerp(const Vector<Len, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            FloatType Length2() const { return Dot(*this); }

            FloatType Length() const { return sqrtf(Length2()); }

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<Len, T> Normal() const
            {
                FloatType s = Length();
                return s == 0.0 ? (*this) : (*this) * (FloatType(1.0) / s);
            }
        };

        template <size_t Len, typename T>
        inline const Vector<Len, T> Vector<Len, T>::ZERO = Vector<Len, T>(0);

        template <size_t Len, typename T>
        inline const Vector<Len, T> Vector<Len, T>::ONE = Vector<Len, T>(1);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector2
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<2, T>
        {
            static inline constexpr size_t SIZE = 2;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<SIZE, T> ZERO;
            static const Vector<SIZE, T> ONE;
            static const Vector<SIZE, T> UNIT_X;
            static const Vector<SIZE, T> UNIT_Y;

            constexpr Vector() = default;
            constexpr Vector(T s) : x(s), y(s) { }
            constexpr Vector(T x, T y) : x(x), y(y) { }

            template <typename U>
            constexpr Vector(const Vector<SIZE, U>& vector) : x(static_cast<T>(vector.x)),
                                                              y(static_cast<T>(vector.y))
            {
            }

            bool operator==(const Vector<SIZE, T>& v) const { return x == v.x && y == v.y; }
            bool operator!=(const Vector<SIZE, T>& v) const { return !(*this == v); }
            bool operator==(T s) const { return x == s && y == s; }
            bool operator!=(T s) const { return !(*this == s); }
            bool operator<(const Vector<SIZE, T>& v) const { return x < v.x && y < v.y; }
            bool operator>(const Vector<SIZE, T>& v) const { return x > v.x && y > v.y; }

            Vector<SIZE, T> operator-() const { return Vector<SIZE, T>(-x, -y); }

            Vector<SIZE, T>& operator+=(const Vector<SIZE, T>& v)
            {
                x += v.x;
                y += v.y;
                return *this;
            }

            Vector<SIZE, T>& operator-=(const Vector<SIZE, T>& v)
            {
                x -= v.x;
                y -= v.y;
                return *this;
            }

            Vector<SIZE, T>& operator*=(const Vector<SIZE, T>& v)
            {
                x *= v.x;
                y *= v.y;
                return *this;
            }

            Vector<SIZE, T>& operator/=(const Vector<SIZE, T>& v)
            {
                x /= v.x;
                y /= v.y;
                return *this;
            }

            Vector<SIZE, T>& operator+=(T s)
            {
                x += s;
                y += s;
                return *this;
            }

            Vector<SIZE, T>& operator-=(T s)
            {
                x -= s;
                y -= s;
                return *this;
            }

            Vector<SIZE, T>& operator*=(T s)
            {
                x *= s;
                y *= s;
                return *this;
            }

            Vector<SIZE, T>& operator/=(T s)
            {
                x /= s;
                y /= s;
                return *this;
            }

            Vector<SIZE, T> operator+(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x + v.x, y + v.y); }
            Vector<SIZE, T> operator-(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x - v.x, y - v.y); }
            Vector<SIZE, T> operator*(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x * v.x, y * v.y); }
            Vector<SIZE, T> operator/(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x / v.x, y / v.y); }
            Vector<SIZE, T> operator+(T s) const { return Vector<SIZE, T>(x + s, y + s); }
            Vector<SIZE, T> operator-(T s) const { return Vector<SIZE, T>(x - s, y - s); }
            Vector<SIZE, T> operator*(T s) const { return Vector<SIZE, T>(x * s, y * s); }
            Vector<SIZE, T> operator/(T s) const { return Vector<SIZE, T>(x / s, y / s); }

            T Dot(const Vector<SIZE, T>& v) const { return x * v.x + y * v.y; }
            T Cross(const Vector<SIZE, T>& v) const { return x * v.y - y * v.x; }
            Vector<SIZE, T> Abs() const { return Vector<SIZE, T>(fabsf(x), fabsf(y)); }

            Vector<SIZE, T>& Rotate(const Vector<SIZE, T>& cs)
            {
                *this = Vector<SIZE, T>(x * cs.x - y * cs.y, x * cs.y + y * cs.x);
                return *this;
            }

            Vector<SIZE, T>& Rotate(T angle)
            {
                Vector<SIZE, T> cs;
                sincos(angle, &cs.y, &cs.x);
                return rotate(cs);
            }

            float Angle() const
            {
                return atan2f(y, x);
            }

            // Shared vectors functions
            T& operator[](int index) const;

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const;

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
        };

        template <typename T>
        inline const Vector<2, T> Vector<2, T>::UNIT_X = Vector<2, T>(1, 0);

        template <typename T>
        inline const Vector<2, T> Vector<2, T>::UNIT_Y = Vector<2, T>(0, 1);

        using Vector2 = Vector<2, float>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector3
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<3, T>
        {
            static inline constexpr size_t SIZE = 3;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<SIZE, T> ZERO;
            static const Vector<SIZE, T> ONE;
            static const Vector<SIZE, T> UNIT_X;
            static const Vector<SIZE, T> UNIT_Y;
            static const Vector<SIZE, T> UNIT_Z;

            constexpr Vector() = default;
            constexpr Vector(T s) : x(s), y(s), z(s) { }
            constexpr Vector(T x, T y, T z) : x(x), y(y), z(z) { }

            template <typename U>
            constexpr Vector(const Vector<SIZE, U>& vector) : x(static_cast<T>(vector.x)),
                                                              y(static_cast<T>(vector.y)),
                                                              z(static_cast<T>(vector.z))
            {
            }

            constexpr Vector(const Vector<SIZE - 1, T>& xy, float z) : x(xy.x), y(xy.y), z(z) { }
            constexpr Vector(float lng, float lat) : x(sinf(lat) * cosf(lng)), y(-sinf(lng)), z(cosf(lat) * cosf(lng)) { }

            Vector<SIZE - 1, T>& xy() const { return *((Vector<SIZE - 1, T>*)&x); }
            Vector<SIZE - 1, T>& yz() const { return *((Vector<SIZE - 1, T>*)&y); }

            bool operator==(const Vector<SIZE, T>& v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const Vector<SIZE, T>& v) const { return !(*this == v); }
            bool operator==(T s) const { return x == s && y == s && z == s; }
            bool operator!=(T s) const { return !(*this == s); }
            bool operator<(const Vector<SIZE, T>& v) const { return x < v.x && y < v.y && z < v.z; }
            bool operator>(const Vector<SIZE, T>& v) const { return x > v.x && y > v.y && z > v.z; }

            Vector<SIZE, T> operator-() const { return Vector<SIZE, T>(-x, -y, -z); }

            Vector<SIZE, T>& operator+=(const Vector<SIZE, T>& v)
            {
                x += v.x;
                y += v.y;
                z += v.z;
                return *this;
            }

            Vector<SIZE, T>& operator-=(const Vector<SIZE, T>& v)
            {
                x -= v.x;
                y -= v.y;
                z -= v.z;
                return *this;
            }

            Vector<SIZE, T>& operator*=(const Vector<SIZE, T>& v)
            {
                x *= v.x;
                y *= v.y;
                z *= v.z;
                return *this;
            }

            Vector<SIZE, T>& operator/=(const Vector<SIZE, T>& v)
            {
                x /= v.x;
                y /= v.y;
                z /= v.z;
                return *this;
            }

            Vector<SIZE, T>& operator+=(T s)
            {
                x += s;
                y += s;
                z += s;
                return *this;
            }

            Vector<SIZE, T>& operator-=(T s)
            {
                x -= s;
                y -= s;
                z -= s;
                return *this;
            }

            Vector<SIZE, T>& operator*=(T s)
            {
                x *= s;
                y *= s;
                z *= s;
                return *this;
            }

            Vector<SIZE, T>& operator/=(T s)
            {
                x /= s;
                y /= s;
                z /= s;
                return *this;
            }

            Vector<SIZE, T> operator+(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x + v.x, y + v.y, z + v.z); }
            Vector<SIZE, T> operator-(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x - v.x, y - v.y, z - v.z); }
            Vector<SIZE, T> operator*(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x * v.x, y * v.y, z * v.z); }
            Vector<SIZE, T> operator/(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x / v.x, y / v.y, z / v.z); }
            Vector<SIZE, T> operator+(T s) const { return Vector<SIZE, T>(x + s, y + s, z + s); }
            Vector<SIZE, T> operator-(T s) const { return Vector<SIZE, T>(x - s, y - s, z - s); }
            Vector<SIZE, T> operator*(T s) const { return Vector<SIZE, T>(x * s, y * s, z * s); }
            Vector<SIZE, T> operator/(T s) const { return Vector<SIZE, T>(x / s, y / s, z / s); }

            T Dot(const Vector<SIZE, T>& v) const { return x * v.x + y * v.y + z * v.z; }
            Vector<SIZE, T> Cross(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
            Vector<SIZE, T> Abs() const { return Vector<SIZE, T>(fabsf(x), fabsf(y), fabsf(z)); }
            Vector<SIZE, T> AxisXZ() const { return (fabsf(x) > fabsf(z)) ? Vector<SIZE, T>(float(Sign(x)), 0, 0) : Vector<SIZE, T>(0, 0, float(Sign(z))); }
            Vector<SIZE, T> Reflect(const Vector<SIZE, T>& n) const { return *this - n * (Dot(n) * 2.0f); }

            const Vector<SIZE, T> RotateY(float angle) const
            {
                float s, c;
                sincos(angle, &s, &c);
                return vec3(x * c - z * s, y, x * s + z * c);
            }

            Radian AngleBetween(const Vector<SIZE, T>& destv) const
            {
                float lenProduct = static_cast<float>(Length() * dest.length());

                // Divide by zero check
                if (lenProduct < std::numeric_limits<float>::epsilon)
                    lenProduct = std::numeric_limits<float>::epsilon;

                float f = Dot(dest) / lenProduct;
                f = Math::Clamp(f, -1.0f, 1.0f);

                return Math::acos(f);
            }

            float AngleX() const { return atan2f(sqrtf(x * x + z * z), y); }

            float AngleY() const { return atan2f(z, x); }

            // Shared vectors functions
            T& operator[](int index) const;

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const;

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
            T z;
        };

        template <typename T>
        inline const Vector<3, T> Vector<3, T>::UNIT_X = Vector<3, T>(1, 0, 0);

        template <typename T>
        inline const Vector<3, T> Vector<3, T>::UNIT_Y = Vector<3, T>(0, 1, 0);

        template <typename T>
        inline const Vector<3, T> Vector<3, T>::UNIT_Z = Vector<3, T>(0, 0, 1);

        using Vector3 = Vector<3, float>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector4
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<4, T>
        {
            static inline constexpr size_t SIZE = 4;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<SIZE, T> ZERO;
            static const Vector<SIZE, T> ONE;
            static const Vector<SIZE, T> UNIT_X;
            static const Vector<SIZE, T> UNIT_Y;
            static const Vector<SIZE, T> UNIT_Z;
            static const Vector<SIZE, T> UNIT_W;

            constexpr Vector() = default;
            constexpr Vector(T s) : x(s), y(s), z(s), w(s) { }
            constexpr Vector(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }

            template <typename U>
            constexpr Vector(const Vector<SIZE, U>& vector) : x(static_cast<T>(vector.x)),
                                                              y(static_cast<T>(vector.y)),
                                                              z(static_cast<T>(vector.z)),
                                                              w(static_cast<T>(vector.w))
            {
            }

            constexpr Vector(const Vector<SIZE - 1, T>& xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) { }
            constexpr Vector(const Vector<SIZE - 2, T>& xy, const Vector<SIZE - 2, T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }

            bool operator==(const Vector<SIZE, T>& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const Vector<SIZE, T>& v) const { return !(*this == v); }
            bool operator==(T s) const { return x == s && y == s && z == s && w == s; }
            bool operator!=(T s) const { return !(*this == s); }
            bool operator<(const Vector<SIZE, T>& v) const { return x < v.x && y < v.y && z < v.z && w < v.w; }
            bool operator>(const Vector<SIZE, T>& v) const { return x > v.x && y > v.y && z > v.z && w > v.w; }

            Vector<SIZE, T> operator-() const { return Vector<SIZE, T>(-x, -y, -z, -w); }

            Vector<SIZE, T>& operator+=(const Vector<SIZE, T>& v)
            {
                x += v.x;
                y += v.y;
                z += v.z;
                w += w.z;
                return *this;
            }

            Vector<SIZE, T>& operator-=(const Vector<SIZE, T>& v)
            {
                x -= v.x;
                y -= v.y;
                z -= v.z;
                w -= v.w;
                return *this;
            }

            Vector<SIZE, T>& operator*=(const Vector<SIZE, T>& v)
            {
                x *= v.x;
                y *= v.y;
                z *= v.z;
                w *= v.w;
                return *this;
            }

            Vector<SIZE, T>& operator/=(const Vector<SIZE, T>& v)
            {
                x /= v.x;
                y /= v.y;
                z /= v.z;
                w /= v.w;
                return *this;
            }

            Vector<SIZE, T>& operator+=(T s)
            {
                x += s;
                y += s;
                z += s;
                w += s;
                return *this;
            }

            Vector<SIZE, T>& operator-=(T s)
            {
                x -= s;
                y -= s;
                z -= s;
                w -= s;
                return *this;
            }

            Vector<SIZE, T>& operator*=(T s)
            {
                x *= s;
                y *= s;
                z *= s;
                w *= s;
                return *this;
            }

            Vector<SIZE, T>& operator/=(T s)
            {
                x /= s;
                y /= s;
                z /= s;
                w /= s;
                return *this;
            }

            Vector<SIZE, T> operator+(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x + v.x, y + v.y, z + v.z, w + v.w); }
            Vector<SIZE, T> operator-(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x - v.x, y - v.y, z - v.z, w - v.w); }
            Vector<SIZE, T> operator*(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x * v.x, y * v.y, z * v.z, w * v.w); }
            Vector<SIZE, T> operator/(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(x / v.x, y / v.y, z / v.z, w / v.w); }
            Vector<SIZE, T> operator+(T s) const { return Vector<SIZE, T>(x + s, y + s, z + s, w + s); }
            Vector<SIZE, T> operator-(T s) const { return Vector<SIZE, T>(x - s, y - s, z - s, w - s); }
            Vector<SIZE, T> operator*(T s) const { return Vector<SIZE, T>(x * s, y * s, z * s, w * s); }
            Vector<SIZE, T> operator/(T s) const { return Vector<SIZE, T>(x / s, y / s, z / s, w / s); }

            T Dot(const Vector<SIZE, T>& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
            Vector<SIZE, T> Abs() const { return Vector<SIZE, T>(fabsf(x), fabsf(y), fabsf(z), fabsf(w)); }

            // Shared vectors functions
            T& operator[](int index) const;

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const;

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
            T z;
            T w;
        };

        template <typename T>
        inline const Vector<4, T> Vector<4, T>::UNIT_X = Vector<4, T>(1, 0, 0, 0);

        template <typename T>
        inline const Vector<4, T> Vector<4, T>::UNIT_Y = Vector<4, T>(0, 1, 0, 0);

        template <typename T>
        inline const Vector<4, T> Vector<4, T>::UNIT_Z = Vector<4, T>(0, 0, 1, 0);

        template <typename T>
        inline const Vector<4, T> Vector<4, T>::UNIT_W = Vector<4, T>(0, 0, 0, 1);

        using Vector4 = Vector<4, float>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Rect
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Rect
        {
            Rect() = default;
            Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight)
                : left(rectLeft), top(rectTop), width(rectWidth), height(rectHeight) {};

            Rect(const Vector<2, T>& position, const Vector<2, T>& size)
                : left(position.x), top(position.y), width(size.x), height(size.y)
            {
            }

            template <typename U>
            explicit Rect(const Rect<U>& rectangle)
                : left(static_cast<T>(rectangle.left)), top(static_cast<T>(rectangle.top)), width(static_cast<T>(rectangle.width)), height(static_cast<T>(rectangle.height)) {};

            bool Contains(T x, T y) const;
            bool Contains(const Vector<2, T>& point) const;
            bool Intersects(const Rect<T>& rect) const;
            bool Intersects(const Rect<T>& rect, Rect<T>& intersection) const;

            Vector<2, T> GetPosition() const;
            Vector<2, T> GetSize() const;

            bool operator==(const Rect<T>& rect) const;
            bool operator!=(const Rect<T>& rect) const;

            T left;
            T top;
            T width;
            T height;
        };

        template <typename T>
        bool Rect<T>::Contains(T x, T y) const
        {
            // Rectangles with negative dimensions are allowed, so we must handle them correctly
            // Compute the real min and max of the rectangle on both axes

            T minX = std::min(left, static_cast<T>(left + width));
            T maxX = std::max(left, static_cast<T>(left + width));
            T minY = std::min(top, static_cast<T>(top + height));
            T maxY = std::max(top, static_cast<T>(top + height));

            return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
        }

        template <typename T>
        bool Rect<T>::Contains(const Vector<2, T>& point) const
        {
            return contains(point.x, point.y);
        }

        template <typename T>
        bool Rect<T>::Intersects(const Rect<T>& rect) const
        {
            Rect<T> intersection;
            return intersects(rect, intersection);
        }

        template <typename T>
        bool Rect<T>::Intersects(const Rect<T>& rect, Rect<T>& intersection) const
        {
            // Rectangles with negative dimensions are allowed, so we must handle them correctly

            // Compute the min and max of the first rectangle on both axes
            T r1MinX = std::min(left, static_cast<T>(left + width));
            T r1MaxX = std::max(left, static_cast<T>(left + width));
            T r1MinY = std::min(top, static_cast<T>(top + height));
            T r1MaxY = std::max(top, static_cast<T>(top + height));

            // Compute the min and max of the second rectangle on both axes
            T r2MinX = std::min(rect.left, static_cast<T>(rect.left + rect.width));
            T r2MaxX = std::max(rect.left, static_cast<T>(rect.left + rect.width));
            T r2MinY = std::min(rect.top, static_cast<T>(rect.top + rect.height));
            T r2MaxY = std::max(rect.top, static_cast<T>(rect.top + rect.height));

            // Compute the intersection boundaries
            T interLeft = std::max(r1MinX, r2MinX);
            T interTop = std::max(r1MinY, r2MinY);
            T interRight = std::min(r1MaxX, r2MaxX);
            T interBottom = std::min(r1MaxY, r2MaxY);

            // If the intersection is valid (positive non zero area), then there is an intersection
            if ((interLeft < interRight) && (interTop < interBottom))
            {
                intersection = Rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
                return true;
            }
            else
            {
                intersection = Rect<T>(0, 0, 0, 0);
                return false;
            }
        }

        template <typename T>
        Vector<2, T> Rect<T>::GetPosition() const
        {
            return Vector<2, T>(left, top);
        }

        template <typename T>
        Vector<2, T> Rect<T>::GetSize() const
        {
            return Vector<2, T>(width, height);
        }

        template <typename T>
        bool Rect<T>::operator==(const Rect<T>& rect) const
        {
            return (left == rect.left) && (width == rect.width) && (top == rect.top) && (height == rect.height);
        }

        template <typename T>
        bool Rect<T>::operator!=(const Rect<T>& rect) const
        {
            return !(left == rect);
        }

        using AlignedBox2i = Rect<uint32_t>;

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