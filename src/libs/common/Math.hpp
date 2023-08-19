#pragma once

#include <algorithm>
#include <cmath>

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace RR
{
    namespace Common
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

            template <AngleUnitType T, typename =  std::enable_if_t<UT != T, void>>
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
        // Vector
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <size_t Len, typename T>
        struct Vector
        {
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector2
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<2, T>
        {
            static inline constexpr size_t SIZE = 2;
            using FloatFormat = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

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

            template <typename U>
            Vector<SIZE, U> Cast() const
            {
                return Vector<SIZE, U>(
                    static_cast<U>(x),
                    static_cast<U>(y));
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

            Vector<SIZE, T>& Rotate(Radian angle)
            {
                Vector<SIZE, T> cs;
                SinCos(angle.Value(), &cs.y, &cs.x);
                return Vector<SIZE, T>(x * cs.x - y * cs.y, x * cs.y + y * cs.x);
            }

            Radian Angle() const
            {
                return atan2f(y, x);
            }

            // Shared vectors functions
            T& operator[](int index) const
            {
                ASSERT(index >= 0 && index < SIZE)
                return ((T*)this)[index];
            }

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            FloatFormat LengthSqr() const { return Dot(*this); }

            FloatFormat Length() const { return sqrtf(LengthSqr()); }

            template <bool isFloatPoint = std::is_floating_point<T>::value, typename = std::enable_if_t<isFloatPoint>>
            Vector<SIZE, T> Normal() const
            {
                FloatFormat s = Length();
                return s == 0.0 ? (*this) : (*this) * (FloatFormat(1.0) / s);
            }
            ////////////////////////////////

            T x;
            T y;
        };

        template <typename T>
        inline const Vector<Vector<2, T>::SIZE, T> Vector<2, T>::ZERO = Vector<2, T>(0);

        template <typename T>
        inline const Vector<Vector<2, T>::SIZE, T> Vector<2, T>::ONE = Vector<2, T>(1);

        template <typename T>
        inline const Vector<Vector<2, T>::SIZE, T> Vector<2, T>::UNIT_X = Vector<2, T>(1, 0);

        template <typename T>
        inline const Vector<Vector<2, T>::SIZE, T> Vector<2, T>::UNIT_Y = Vector<2, T>(0, 1);

        using Vector2 = Vector<2, float>;
        using Vector2i = Vector<2, int32_t>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector3
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<3, T>
        {
            static inline constexpr size_t SIZE = 3;
            using FloatFormat = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<SIZE, T> ZERO;
            static const Vector<SIZE, T> ONE;
            static const Vector<SIZE, T> UNIT_X;
            static const Vector<SIZE, T> UNIT_Y;
            static const Vector<SIZE, T> UNIT_Z;
            static const Vector<SIZE, T> UP;
            static const Vector<SIZE, T> FORWARD;

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

            template <typename U>
            Vector<SIZE, U> Cast() const
            {
                return Vector<SIZE, U>(
                    static_cast<U>(x),
                    static_cast<U>(y),
                    static_cast<U>(z));
            }

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

            const Vector<SIZE, T> RotateY(Radian angle) const
            {
                float s, c;
                SinCos(angle.Value(), &s, &c);
                return vec3(x * c - z * s, y, x * s + z * c);
            }

            Radian AngleBetween(const Vector<SIZE, T>& dest) const
            {
                float lenProduct = static_cast<float>(Length() * dest.length());

                // Divide by zero check
                if (lenProduct < std::numeric_limits<float>::epsilon())
                    lenProduct = std::numeric_limits<float>::epsilon();

                float f = Dot(dest) / lenProduct;
                f = Clamp(f, -1.0f, 1.0f);

                return Acos(f);
            }

            Radian AngleX() const { return atan2f(sqrtf(x * x + z * z), y); }

            Radian AngleY() const { return atan2f(z, x); }

            // Shared vectors functions
            T& operator[](int index) const
            {
                ASSERT(index >= 0 && index < SIZE)
                return ((T*)this)[index];
            }

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            FloatFormat LengthSqr() const { return Dot(*this); }

            FloatFormat Length() const { return sqrtf(LengthSqr()); }

            template <bool isFloatPoint = std::is_floating_point<T>::value, typename = std::enable_if_t<isFloatPoint>>
            Vector<SIZE, T> Normal() const
            {
                FloatFormat s = Length();
                return s == 0.0 ? (*this) : (*this) * (FloatFormat(1.0) / s);
            }
            ////////////////////////////////

            T x;
            T y;
            T z;
        };

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::ZERO = Vector<3, T>(0);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::ONE = Vector<3, T>(1);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::UNIT_X = Vector<3, T>(1, 0, 0);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::UNIT_Y = Vector<3, T>(0, 1, 0);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::UNIT_Z = Vector<3, T>(0, 0, 1);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::UP = Vector<3, T>(0, 1, 0);

        template <typename T>
        inline const Vector<Vector<3, T>::SIZE, T> Vector<3, T>::FORWARD = Vector<3, T>(0, 0, 1);

        using Vector3 = Vector<3, float>;
        using Vector3u = Vector<3, uint32_t>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector4
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<4, T>
        {
            static inline constexpr size_t SIZE = 4;
            using FloatFormat = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            static const Vector<SIZE, T> ZERO;
            static const Vector<SIZE, T> ONE;
            static const Vector<SIZE, T> UNIT_X;
            static const Vector<SIZE, T> UNIT_Y;
            static const Vector<SIZE, T> UNIT_Z;
            static const Vector<SIZE, T> UNIT_W;

            constexpr Vector() = default;
            constexpr Vector(T s) : x(s), y(s), z(s), w(s) { }
            constexpr Vector(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }
            constexpr Vector(const Vector<SIZE - 1, T>& xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) { }
            constexpr Vector(const Vector<SIZE - 2, T>& xy, const Vector<SIZE - 2, T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }

            Vector<SIZE - 1, T>& xyz() const { return *((Vector<SIZE - 1, T>*)&x); }

            template <typename U>
            Vector<SIZE, U> Cast() const
            {
                return Vector<SIZE, U>(
                    static_cast<U>(x),
                    static_cast<U>(y),
                    static_cast<U>(z),
                    static_cast<U>(w));
            }

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
            T& operator[](int index) const
            {
                ASSERT(index >= 0 && index < SIZE)
                return ((T*)this)[index];
            }

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            FloatFormat LengthSqr() const { return Dot(*this); }

            FloatFormat Length() const { return sqrtf(LengthSqr()); }

            template <typename = std::enable_if_t<std::is_floating_point<T>::value>>
            Vector<SIZE, T> Normal() const
            {
                FloatFormat s = Length();
                return s == 0.0 ? (*this) : (*this) * (FloatFormat(1.0) / s);
            }
            ////////////////////////////////

            T x;
            T y;
            T z;
            T w;
        };

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::ZERO = Vector<4, T>(0);

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::ONE = Vector<4, T>(1);

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::UNIT_X = Vector<4, T>(1, 0, 0, 0);

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::UNIT_Y = Vector<4, T>(0, 1, 0, 0);

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::UNIT_Z = Vector<4, T>(0, 0, 1, 0);

        template <typename T>
        inline const Vector<Vector<4, T>::SIZE, T> Vector<4, T>::UNIT_W = Vector<4, T>(0, 0, 0, 1);

        using Vector4 = Vector<4, float>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Quaternion
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        struct Quaternion
        {
            using FloatFormat = float;
            static_assert(std::is_floating_point<FloatFormat>::value, "Float point type assumed");
            static_assert(std::is_same<FloatFormat, float>::value, "Only float are supported now");

            Vector<3, FloatFormat>& xyz() const { return *((Vector<3, FloatFormat>*)&x); }

            Quaternion() = default;

            Quaternion(FloatFormat x, FloatFormat y, FloatFormat z, FloatFormat w)
                : x(x), y(y), z(z), w(w)
            {
            }

            Quaternion(const Vector<3, FloatFormat>& axis, Radian angle)
            {
                float s, c;
                SinCos(angle.Value() * 0.5f, &s, &c);
                x = axis.x * s;
                y = axis.y * s;
                z = axis.z * s;
                w = c;
            }

            Quaternion(const Vector<3, FloatFormat>& lookAt)
            {
                Vector<3, FloatFormat> forward = lookAt.Normal();
                Vector<3, FloatFormat> rotAxis = Vector<3, FloatFormat>::UNIT_Z.Cross(forward);
                float dot = Vector<3, FloatFormat>::UNIT_Z.Dot(forward);

                x = rotAxis.x;
                y = rotAxis.y;
                z = rotAxis.z;
                w = dot + 1;

                Normalize();
            }

            Quaternion(const Vector<3, FloatFormat>& lookAt, const Vector<3, FloatFormat>& upVector)
            {
                Vector<3, FloatFormat> up = upVector.Normal();
                Vector<3, FloatFormat> forward = lookAt.Normal();
                Vector<3, FloatFormat> right = up.Cross(forward).Normal();
                up = forward.Cross(right);

                float e00 = right.x, e10 = right.y, e20 = right.z;
                float e01 = up.x, e11 = up.y, e21 = up.z;
                float e02 = forward.x, e12 = forward.y, e22 = forward.z;

                float t, s;
                t = 1.0f + e00 + e11 + e22;
                if (t > 0.0001f)
                {
                    s = 0.5f / sqrtf(t);
                    x = (e21 - e12) * s;
                    y = (e02 - e20) * s;
                    z = (e10 - e01) * s;
                    w = 0.25f / s;
                }
                else if (e00 > e11 && e00 > e22)
                {
                    s = 0.5f / sqrtf(1.0f + e00 - e11 - e22);
                    x = 0.25f / s;
                    y = (e01 + e10) * s;
                    z = (e02 + e20) * s;
                    w = (e21 - e12) * s;
                }
                else if (e11 > e22)
                {
                    s = 0.5f / sqrtf(1.0f - e00 + e11 - e22);
                    x = (e01 + e10) * s;
                    y = 0.25f / s;
                    z = (e12 + e21) * s;
                    w = (e02 - e20) * s;
                }
                else
                {
                    s = 0.5f / sqrtf(1.0f - e00 - e11 + e22);
                    x = (e02 + e20) * s;
                    y = (e12 + e21) * s;
                    z = 0.25f / s;
                    w = (e10 - e01) * s;
                }
            }

            Quaternion operator-() const
            {
                return Quaternion(-x, -y, -z, -w);
            }

            Quaternion operator+(const Quaternion& q) const
            {
                return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
            }

            Quaternion operator-(const Quaternion& q) const
            {
                return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
            }

            Quaternion operator*(const FloatFormat s) const
            {
                return Quaternion(x * s, y * s, z * s, w * s);
            }

            Quaternion operator*(const Quaternion& q) const
            {
                return Quaternion(w * q.x + x * q.w + y * q.z - z * q.y,
                                  w * q.y + y * q.w + z * q.x - x * q.z,
                                  w * q.z + z * q.w + x * q.y - y * q.x,
                                  w * q.w - x * q.x - y * q.y - z * q.z);
            }

            Vector<3, FloatFormat> operator*(const Vector<3, FloatFormat>& v) const
            {
                //return v + xyz.cross(xyz.cross(v) + v * w) * 2.0f;
                return (*this * Quaternion(v.x, v.y, v.z, 0) * Inverse()).xyz();
            }

            float Dot(const Quaternion& q) const
            {
                return x * q.x + y * q.y + z * q.z + w * q.w;
            }

            float LengthSqr() const
            {
                return Dot(*this);
            }

            float Length() const
            {
                return sqrtf(LengthSqr());
            }

            void Normalize()
            {
                *this = Normal();
            }

            Quaternion Normal() const
            {
                const float l = Length();
                return l == 0.0 ? (*this) : (*this) * (1.0f / l);
            }

            Quaternion Conjugate() const
            {
                return Quaternion(-x, -y, -z, w);
            }

            Quaternion Inverse() const
            {
                const float l2 = LengthSqr();
                const float l2inv = l2 == 0.0f ? 0.0f : (1.0f / l2);

                return Conjugate() * l2inv;
            }

            Quaternion Lerp(const Quaternion& q, float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return q;

                return Dot(q) < 0 ? (*this - (q + *this) * t) : (*this + (q - *this) * t);
            }

            Quaternion Slerp(const Quaternion& q, float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return q;

                Quaternion temp;
                float omega, cosom, sinom, scale0, scale1;

                cosom = Dot(q);
                if (cosom < 0.0f)
                {
                    temp = -q;
                    cosom = -cosom;
                }
                else
                    temp = q;

                if (1.0f - cosom > EPS)
                {
                    omega = acosf(cosom);
                    sinom = 1.0f / sinf(omega);
                    scale0 = sinf((1.0f - t) * omega) * sinom;
                    scale1 = sinf(t * omega) * sinom;
                }
                else
                {
                    scale0 = 1.0f - t;
                    scale1 = t;
                }

                return *this * scale0 + temp * scale1;
            }

            FloatFormat x, y, z, w;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Matrix
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <size_t M, size_t K>
        struct Matrix;

        template <>
        struct Matrix<4, 4>
        {
            static inline constexpr size_t M = 4;
            static inline constexpr size_t K = 4;
            using FloatFormat = float;
            static_assert(std::is_floating_point<FloatFormat>::value, "Float point type assumed");
            static_assert(std::is_same<FloatFormat, float>::value, "Only float are supported now");

            enum ProjRange
            {
                PROJ_NEG_POS,
                PROJ_ZERO_POS,
            };

            Vector<3, FloatFormat>& Right() const { return *((Vector<3, FloatFormat>*)&e00); }

            Vector<3, FloatFormat>& Up() const { return *((Vector<3, FloatFormat>*)&e01); }

            Vector<3, FloatFormat>& Forward() const { return *((Vector<3, FloatFormat>*)&e02); }

            Vector<4, FloatFormat>& Offset() const { return *((Vector<4, FloatFormat>*)&e03); }

            Matrix() = default;

            Matrix(Initialization)
            {
                Identity();
            }

            Matrix(FloatFormat e00, FloatFormat e10, FloatFormat e20, FloatFormat e30,
                   FloatFormat e01, FloatFormat e11, FloatFormat e21, FloatFormat e31,
                   FloatFormat e02, FloatFormat e12, FloatFormat e22, FloatFormat e32,
                   FloatFormat e03, FloatFormat e13, FloatFormat e23, FloatFormat e33)
                : e00(e00), e10(e10), e20(e20), e30(e30), e01(e01), e11(e11), e21(e21), e31(e31), e02(e02), e12(e12), e22(e22), e32(e32), e03(e03), e13(e13), e23(e23), e33(e33)
            {
            }

            Matrix(const Quaternion& rotation, const Vector<3, FloatFormat>& position)
            {
                SetRot(rotation);
                SetPos(position);
                e30 = e31 = e32 = 0.0f;
                e33 = 1.0f;
            }

            Matrix(ProjRange range, FloatFormat l, FloatFormat r, FloatFormat b, FloatFormat t, FloatFormat znear, FloatFormat zfar)
            {
                Identity();
                e00 = 2.0f / (r - l);
                e11 = 2.0f / (t - b);
                e22 = 2.0f / (znear - zfar);
                e03 = (l + r) / (l - r);
                e13 = (t + b) / (b - t);
                switch (range)
                {
                case PROJ_NEG_POS:
                    e23 = (zfar + znear) / (znear - zfar);
                    break;
                case PROJ_ZERO_POS:
                    e23 = znear / (znear - zfar);
                    break;
                }
            }

            Matrix(ProjRange range, Radian fov, FloatFormat aspect, FloatFormat znear, FloatFormat zfar)
            {
                FloatFormat k = 1.0f / tanf(fov.Value() * 0.5f);
                Identity();
                if (aspect >= 1.0f)
                {
                    e00 = k / aspect;
                    e11 = k;
                }
                else
                {
                    e00 = k;
                    e11 = k * aspect;
                }
                e33 = 0.0f;
                e32 = -1.0f;
                switch (range)
                {
                case PROJ_NEG_POS:
                    e22 = (znear + zfar) / (znear - zfar);
                    e23 = 2.0f * zfar * znear / (znear - zfar);
                    break;
                case PROJ_ZERO_POS:
                    e22 = zfar / (znear - zfar);
                    e23 = znear * e22;
                    break;
                }
            }

            Matrix(const Vector<3, FloatFormat>& from, const Vector<3, FloatFormat>& at, const Vector<3, FloatFormat>& up)
            {
                Vector<3, FloatFormat> r, u, d;
                d = (from - at).Normal();
                r = up.Cross(d).Normal();
                u = d.Cross(r);

                this->Right() = r;
                this->Up() = u;
                this->Forward() = d;
                this->Offset() = Vector<4, FloatFormat>(from, 1.0f);
                e30 = e31 = e32 = 0;
            }

            Matrix(const Vector<4, FloatFormat>& reflectPlane)
            {
                FloatFormat a = reflectPlane.x,
                            b = reflectPlane.y,
                            c = reflectPlane.z,
                            d = reflectPlane.w;

                Right() = Vector<3, FloatFormat>(1 - 2 * a * a, -2 * b * a, -2 * c * a);
                Up() = Vector<3, FloatFormat>(-2 * a * b, 1 - 2 * b * b, -2 * c * b);
                Forward() = Vector<3, FloatFormat>(-2 * a * c, -2 * b * c, 1 - 2 * c * c);
                Offset() = Vector<4, FloatFormat>(-2 * a * d, -2 * b * d, -2 * c * d, 1);
                e30 = e31 = e32 = 0;
            }

            void Identity()
            {
                e10 = e20 = e30 = e01 = e21 = e31 = e02 = e12 = e32 = e03 = e13 = e23 = 0.0f;
                e00 = e11 = e22 = e33 = 1.0f;
            }

            Matrix<M, K> operator*(const Matrix<M, K>& m) const
            {
                Matrix<M, K> r;
                r.e00 = e00 * m.e00 + e01 * m.e10 + e02 * m.e20 + e03 * m.e30;
                r.e10 = e10 * m.e00 + e11 * m.e10 + e12 * m.e20 + e13 * m.e30;
                r.e20 = e20 * m.e00 + e21 * m.e10 + e22 * m.e20 + e23 * m.e30;
                r.e30 = e30 * m.e00 + e31 * m.e10 + e32 * m.e20 + e33 * m.e30;
                r.e01 = e00 * m.e01 + e01 * m.e11 + e02 * m.e21 + e03 * m.e31;
                r.e11 = e10 * m.e01 + e11 * m.e11 + e12 * m.e21 + e13 * m.e31;
                r.e21 = e20 * m.e01 + e21 * m.e11 + e22 * m.e21 + e23 * m.e31;
                r.e31 = e30 * m.e01 + e31 * m.e11 + e32 * m.e21 + e33 * m.e31;
                r.e02 = e00 * m.e02 + e01 * m.e12 + e02 * m.e22 + e03 * m.e32;
                r.e12 = e10 * m.e02 + e11 * m.e12 + e12 * m.e22 + e13 * m.e32;
                r.e22 = e20 * m.e02 + e21 * m.e12 + e22 * m.e22 + e23 * m.e32;
                r.e32 = e30 * m.e02 + e31 * m.e12 + e32 * m.e22 + e33 * m.e32;
                r.e03 = e00 * m.e03 + e01 * m.e13 + e02 * m.e23 + e03 * m.e33;
                r.e13 = e10 * m.e03 + e11 * m.e13 + e12 * m.e23 + e13 * m.e33;
                r.e23 = e20 * m.e03 + e21 * m.e13 + e22 * m.e23 + e23 * m.e33;
                r.e33 = e30 * m.e03 + e31 * m.e13 + e32 * m.e23 + e33 * m.e33;
                return r;
            }

            Vector<3, FloatFormat> operator*(const Vector<3, FloatFormat>& v) const
            {
                return Vector<3, FloatFormat>(
                    e00 * v.x + e01 * v.y + e02 * v.z + e03,
                    e10 * v.x + e11 * v.y + e12 * v.z + e13,
                    e20 * v.x + e21 * v.y + e22 * v.z + e23);
            }

            Vector<4, FloatFormat> operator*(const Vector<4, FloatFormat>& v) const
            {
                return Vector<4, FloatFormat>(
                    e00 * v.x + e01 * v.y + e02 * v.z + e03 * v.w,
                    e10 * v.x + e11 * v.y + e12 * v.z + e13 * v.w,
                    e20 * v.x + e21 * v.y + e22 * v.z + e23 * v.w,
                    e30 * v.x + e31 * v.y + e32 * v.z + e33 * v.w);
            }

            void Translate(const Vector<3, FloatFormat>& offset)
            {
                Matrix<M, K> m;
                m.Identity();
                m.SetPos(offset);
                *this = *this * m;
            };

            void Scale(const Vector<3, FloatFormat>& factor)
            {
                Matrix<M, K> m;
                m.Identity();
                m.e00 = factor.x;
                m.e11 = factor.y;
                m.e22 = factor.z;
                *this = *this * m;
            }

            void RotateX(Radian angle)
            {
                Matrix<M, K> m;
                m.Identity();
                FloatFormat s, c;
                SinCos(angle.Value(), &s, &c);
                m.e11 = c;
                m.e21 = s;
                m.e12 = -s;
                m.e22 = c;
                *this = *this * m;
            }

            void RotateY(Radian angle)
            {
                Matrix<M, K> m;
                m.Identity();
                FloatFormat s, c;
                SinCos(angle.Value(), &s, &c);
                m.e00 = c;
                m.e20 = -s;
                m.e02 = s;
                m.e22 = c;
                *this = *this * m;
            }

            void RotateZ(Radian angle)
            {
                Matrix<M, K> m;
                m.Identity();
                FloatFormat s, c;
                SinCos(angle.Value(), &s, &c);
                m.e00 = c;
                m.e01 = -s;
                m.e10 = s;
                m.e11 = c;
                *this = *this * m;
            }

            void RotateYXZ(const Vector<3, FloatFormat>& angle)
            {
                FloatFormat s, c, a, b;

                if (angle.y != 0.0f)
                {
                    SinCos(angle.y, &s, &c);

                    a = e00 * c - e02 * s;
                    b = e02 * c + e00 * s;
                    e00 = a;
                    e02 = b;

                    a = e10 * c - e12 * s;
                    b = e12 * c + e10 * s;
                    e10 = a;
                    e12 = b;

                    a = e20 * c - e22 * s;
                    b = e22 * c + e20 * s;
                    e20 = a;
                    e22 = b;
                }

                if (angle.x != 0.0f)
                {
                    SinCos(angle.x, &s, &c);

                    a = e01 * c + e02 * s;
                    b = e02 * c - e01 * s;
                    e01 = a;
                    e02 = b;

                    a = e11 * c + e12 * s;
                    b = e12 * c - e11 * s;
                    e11 = a;
                    e12 = b;

                    a = e21 * c + e22 * s;
                    b = e22 * c - e21 * s;
                    e21 = a;
                    e22 = b;
                }

                if (angle.z != 0.0f)
                {
                    SinCos(angle.z, &s, &c);

                    a = e00 * c + e01 * s;
                    b = e01 * c - e00 * s;
                    e00 = a;
                    e01 = b;

                    a = e10 * c + e11 * s;
                    b = e11 * c - e10 * s;
                    e10 = a;
                    e11 = b;

                    a = e20 * c + e21 * s;
                    b = e21 * c - e20 * s;
                    e20 = a;
                    e21 = b;
                }
            }

            void Lerp(const Matrix<M, K>& m, float t)
            {
                e00 += (m.e00 - e00) * t;
                e01 += (m.e01 - e01) * t;
                e02 += (m.e02 - e02) * t;
                e03 += (m.e03 - e03) * t;

                e10 += (m.e10 - e10) * t;
                e11 += (m.e11 - e11) * t;
                e12 += (m.e12 - e12) * t;
                e13 += (m.e13 - e13) * t;

                e20 += (m.e20 - e20) * t;
                e21 += (m.e21 - e21) * t;
                e22 += (m.e22 - e22) * t;
                e23 += (m.e23 - e23) * t;
            }

            FloatFormat Det() const
            {
                return e00 * (e11 * (e22 * e33 - e32 * e23) - e21 * (e12 * e33 - e32 * e13) + e31 * (e12 * e23 - e22 * e13)) - e10 * (e01 * (e22 * e33 - e32 * e23) - e21 * (e02 * e33 - e32 * e03) + e31 * (e02 * e23 - e22 * e03)) + e20 * (e01 * (e12 * e33 - e32 * e13) - e11 * (e02 * e33 - e32 * e03) + e31 * (e02 * e13 - e12 * e03)) - e30 * (e01 * (e12 * e23 - e22 * e13) - e11 * (e02 * e23 - e22 * e03) + e21 * (e02 * e13 - e12 * e03));
            }

            Matrix<M, K> Inverse() const
            {
                FloatFormat idet = 1.0f / Det();
                Matrix<M, K> r;
                r.e00 = (e11 * (e22 * e33 - e32 * e23) - e21 * (e12 * e33 - e32 * e13) + e31 * (e12 * e23 - e22 * e13)) * idet;
                r.e01 = -(e01 * (e22 * e33 - e32 * e23) - e21 * (e02 * e33 - e32 * e03) + e31 * (e02 * e23 - e22 * e03)) * idet;
                r.e02 = (e01 * (e12 * e33 - e32 * e13) - e11 * (e02 * e33 - e32 * e03) + e31 * (e02 * e13 - e12 * e03)) * idet;
                r.e03 = -(e01 * (e12 * e23 - e22 * e13) - e11 * (e02 * e23 - e22 * e03) + e21 * (e02 * e13 - e12 * e03)) * idet;
                r.e10 = -(e10 * (e22 * e33 - e32 * e23) - e20 * (e12 * e33 - e32 * e13) + e30 * (e12 * e23 - e22 * e13)) * idet;
                r.e11 = (e00 * (e22 * e33 - e32 * e23) - e20 * (e02 * e33 - e32 * e03) + e30 * (e02 * e23 - e22 * e03)) * idet;
                r.e12 = -(e00 * (e12 * e33 - e32 * e13) - e10 * (e02 * e33 - e32 * e03) + e30 * (e02 * e13 - e12 * e03)) * idet;
                r.e13 = (e00 * (e12 * e23 - e22 * e13) - e10 * (e02 * e23 - e22 * e03) + e20 * (e02 * e13 - e12 * e03)) * idet;
                r.e20 = (e10 * (e21 * e33 - e31 * e23) - e20 * (e11 * e33 - e31 * e13) + e30 * (e11 * e23 - e21 * e13)) * idet;
                r.e21 = -(e00 * (e21 * e33 - e31 * e23) - e20 * (e01 * e33 - e31 * e03) + e30 * (e01 * e23 - e21 * e03)) * idet;
                r.e22 = (e00 * (e11 * e33 - e31 * e13) - e10 * (e01 * e33 - e31 * e03) + e30 * (e01 * e13 - e11 * e03)) * idet;
                r.e23 = -(e00 * (e11 * e23 - e21 * e13) - e10 * (e01 * e23 - e21 * e03) + e20 * (e01 * e13 - e11 * e03)) * idet;
                r.e30 = -(e10 * (e21 * e32 - e31 * e22) - e20 * (e11 * e32 - e31 * e12) + e30 * (e11 * e22 - e21 * e12)) * idet;
                r.e31 = (e00 * (e21 * e32 - e31 * e22) - e20 * (e01 * e32 - e31 * e02) + e30 * (e01 * e22 - e21 * e02)) * idet;
                r.e32 = -(e00 * (e11 * e32 - e31 * e12) - e10 * (e01 * e32 - e31 * e02) + e30 * (e01 * e12 - e11 * e02)) * idet;
                r.e33 = (e00 * (e11 * e22 - e21 * e12) - e10 * (e01 * e22 - e21 * e02) + e20 * (e01 * e12 - e11 * e02)) * idet;
                return r;
            }

            Matrix<M, K> InverseOrtho() const
            {
                Matrix<M, K> r;
                r.e00 = e00;
                r.e10 = e01;
                r.e20 = e02;
                r.e30 = 0;
                r.e01 = e10;
                r.e11 = e11;
                r.e21 = e12;
                r.e31 = 0;
                r.e02 = e20;
                r.e12 = e21;
                r.e22 = e22;
                r.e32 = 0;
                r.e03 = -(e03 * e00 + e13 * e10 + e23 * e20); // -dot(pos, right)
                r.e13 = -(e03 * e01 + e13 * e11 + e23 * e21); // -dot(pos, up)
                r.e23 = -(e03 * e02 + e13 * e12 + e23 * e22); // -dot(pos, dir)
                r.e33 = 1;
                return r;
            }

            Matrix<M, K> Transpose() const
            {
                Matrix<M, K> r;
                r.e00 = e00;
                r.e10 = e01;
                r.e20 = e02;
                r.e30 = e03;
                r.e01 = e10;
                r.e11 = e11;
                r.e21 = e12;
                r.e31 = e13;
                r.e02 = e20;
                r.e12 = e21;
                r.e22 = e22;
                r.e32 = e23;
                r.e03 = e30;
                r.e13 = e31;
                r.e23 = e32;
                r.e33 = e33;
                return r;
            }

            Quaternion GetRot() const
            {
                FloatFormat t, s;
                t = 1.0f + e00 + e11 + e22;
                if (t > 0.0001f)
                {
                    s = 0.5f / sqrtf(t);
                    return Quaternion((e21 - e12) * s, (e02 - e20) * s, (e10 - e01) * s, 0.25f / s);
                }
                else if (e00 > e11 && e00 > e22)
                {
                    s = 0.5f / sqrtf(1.0f + e00 - e11 - e22);
                    return Quaternion(0.25f / s, (e01 + e10) * s, (e02 + e20) * s, (e21 - e12) * s);
                }
                else if (e11 > e22)
                {
                    s = 0.5f / sqrtf(1.0f - e00 + e11 - e22);
                    return Quaternion((e01 + e10) * s, 0.25f / s, (e12 + e21) * s, (e02 - e20) * s);
                }
                else
                {
                    s = 0.5f / sqrtf(1.0f - e00 - e11 + e22);
                    return Quaternion((e02 + e20) * s, (e12 + e21) * s, 0.25f / s, (e10 - e01) * s);
                }
            }

            void SetRot(const Quaternion& rot)
            {
                FloatFormat sx = rot.x * rot.x,
                            sy = rot.y * rot.y,
                            sz = rot.z * rot.z,
                            sw = rot.w * rot.w,
                            inv = 1.0f / (sx + sy + sz + sw);

                e00 = (sx - sy - sz + sw) * inv;
                e11 = (-sx + sy - sz + sw) * inv;
                e22 = (-sx - sy + sz + sw) * inv;
                inv *= 2.0f;

                FloatFormat t1 = rot.x * rot.y;
                FloatFormat t2 = rot.z * rot.w;
                e10 = (t1 + t2) * inv;
                e01 = (t1 - t2) * inv;

                t1 = rot.x * rot.z;
                t2 = rot.y * rot.w;
                e20 = (t1 - t2) * inv;
                e02 = (t1 + t2) * inv;

                t1 = rot.y * rot.z;
                t2 = rot.x * rot.w;
                e21 = (t1 + t2) * inv;
                e12 = (t1 - t2) * inv;
            }

            Vector<3, FloatFormat> getPos() const
            {
                return Offset().xyz();
            }

            void SetPos(const Vector<3, FloatFormat>& pos)
            {
                Offset().xyz() = pos;
            }

            FloatFormat e00, e10, e20, e30,
                e01, e11, e21, e31,
                e02, e12, e22, e32,
                e03, e13, e23, e33;
        };

        using Matrix4 = Matrix<4, 4>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Rect
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Rect
        {
            Rect() = default;
            Rect(T left, T top, T width, T height)
                : left(left), top(top), width(width), height(height) {};

            Rect(const Vector<2, T>& position, const Vector<2, T>& size)
                : left(position.x), top(position.y), width(size.x), height(size.y)
            {
            }

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

        using Rect2i = Rect<int32_t>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Box
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Box
        {
            Box() = default;
            Box(T left, T top, T front, T width, T height, T depth)
                : left(left), top(top), front(front), width(width), height(height), depth(depth) {};

            Box(const Vector<3, T>& position, const Vector<3, T>& size)
                : left(position.x), top(position.y), front(position.z),
                  width(size.x), height(size.y), depth(size.z) { }

            bool Contains(T x, T y, T z) const;
            bool Contains(const Vector<3, T>& point) const;
            bool Intersects(const Box<T>& box) const;
            bool Intersects(const Box<T>& box, Box<T>& intersection) const;

            Vector<3, T> GetPosition() const;
            Vector<3, T> GetSize() const;

            bool operator==(const Box<T>& rect) const;
            bool operator!=(const Box<T>& rect) const;

            T left;
            T top;
            T front;
            T width;
            T height;
            T depth;
        };

        template <typename T>
        bool Box<T>::Contains(T x, T y, T z) const
        {
            // Box with negative dimensions are allowed, so we must handle them correctly
            // Compute the real min and max of the box

            T minX = std::min(left, static_cast<T>(left + width));
            T maxX = std::max(left, static_cast<T>(left + width));
            T minY = std::min(top, static_cast<T>(top + height));
            T maxY = std::max(top, static_cast<T>(top + height));
            T minZ = std::min(front, static_cast<T>(front + depth));
            T maxZ = std::max(front, static_cast<T>(front + depth));

            return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY) && (z >= minZ) && (z < maxZ);
        }

        template <typename T>
        bool Box<T>::Contains(const Vector<3, T>& point) const
        {
            return contains(point.x, point.y, point.z);
        }

        template <typename T>
        bool Box<T>::Intersects(const Box<T>& box) const
        {
            Box<T> intersection;
            return intersects(box, intersection);
        }

        template <typename T>
        bool Box<T>::Intersects(const Box<T>& box, Box<T>& intersection) const
        {
            // Box with negative dimensions are allowed, so we must handle them correctly
            // Compute the real min and max of the box

            T r1MinX = std::min(left, static_cast<T>(left + width));
            T r1MaxX = std::max(left, static_cast<T>(left + width));
            T r1MinY = std::min(top, static_cast<T>(top + height));
            T r1MaxY = std::max(top, static_cast<T>(top + height));
            T r1MinZ = std::min(front, static_cast<T>(front + depth));
            T r1MaxZ = std::max(front, static_cast<T>(front + depth));

            // Compute the min and max of the second box
            T r2MinX = std::min(box.left, static_cast<T>(box.left + box.width));
            T r2MaxX = std::max(box.left, static_cast<T>(box.left + box.width));
            T r2MinY = std::min(box.top, static_cast<T>(box.top + box.height));
            T r2MaxY = std::max(box.top, static_cast<T>(box.top + box.height));
            T r2MinZ = std::min(box.front, static_cast<T>(box.front + box.depth));
            T r2MaxZ = std::max(box.front, static_cast<T>(box.front + box.depth));

            // Compute the intersection boundaries
            T interLeft = std::max(r1MinX, r2MinX);
            T interRight = std::min(r1MaxX, r2MaxX);
            T interTop = std::max(r1MinY, r2MinY);
            T interBottom = std::min(r1MaxY, r2MaxY);
            T interFront = std::max(r1MinZ, r2MinZ);
            T interBack = std::min(r1MaxZ, r2MaxZ);

            // If the intersection is valid (positive non zero area), then there is an intersection
            if ((interLeft < interRight) && (interTop < interBottom) && (interFront < interBack))
            {
                intersection = Box<T>(interLeft, interTop, interFront, interRight - interLeft, interBottom - interTop, interBack - interFront);
                return true;
            }
            else
            {
                intersection = Box<T>(0, 0, 0, 0, 0, 0);
                return false;
            }
        }

        template <typename T>
        Vector<3, T> Box<T>::GetPosition() const
        {
            return Vector<3, T>(left, top, front);
        }

        template <typename T>
        Vector<3, T> Box<T>::GetSize() const
        {
            return Vector<3, T>(width, height, depth);
        }

        template <typename T>
        bool Box<T>::operator==(const Box<T>& box) const
        {
            return (left == box.left) && (width == box.width) && (top == box.top) && (height == box.height) && (front == box.front) && (depth == box.depth);
        }

        template <typename T>
        bool Box<T>::operator!=(const Box<T>& box) const
        {
            return !(left == box);
        }

        using Box3u = Box<uint32_t>;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Align
        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        template <typename T>
        inline constexpr T AlignTo(T value, size_t alignment)
        {
            static_assert(std::is_integral<T>::value, "Expect integral types.");
            ASSERT(IsPowerOfTwo(alignment))
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
}