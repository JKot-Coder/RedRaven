#pragma once

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

#include "common/Utils.hpp"

#define DECL_ENUM(v) v,
#define DECL_STR(v)  #v,

#define EPS     FLT_EPSILON
#define INF     INFINITY
#define PI      3.14159265358979323846f
#define PI2     (PI * 2.0f)
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#define COS30   0.86602540378f
#define COS45   0.70710678118f
#define COS60   0.50000000000f

#define SQR(x)  ((x)*(x))
#define randf() ((float)rand()/RAND_MAX)

#if defined(min) | defined(max)
#undef min
#undef max
#endif

namespace Common {

    template<typename T>
    inline constexpr T min(const T a, const T b) {
        return a < b ? a : b;
    }

    template<typename T>
    inline constexpr T min(const T a, const T b, const T c) {
        return (a < b && a < c) ? a : ((b < a && b < c) ? b : c);
    }

    template<class T>
    inline constexpr T max(const T a, const T b) {
        return a > b ? a : b;
    }

    template <typename T> 
    inline constexpr T max( const T a, const T b, const T c )
	{
        return (a > b && a > c) ? a : ((b > a && b > c) ? b : c);
    }

    template<class T>
    inline constexpr T clamp(const T x, const T a, const T b) {
        return x < a ? a : (x > b ? b : x);
    }

    template<typename T>
    inline constexpr int sign(const T val) {
        return (T(0) < val) - (val < T(0));
    }  

    inline void sincos(float r, float *s, float *c) {
        *s = sinf(r);
        *c = cosf(r);
    }
  
    struct vec2 {
        float x, y;

        vec2() {}

        vec2(float s) : x(s), y(s) {}

        vec2(float x, float y) : x(x), y(y) {}

        inline float &operator[](int index) const {
            ASSERT(index >= 0 && index <= 1);
            return ((float *) this)[index];
        }

        inline bool operator==(const vec2 &v) const { return x == v.x && y == v.y; }

        inline bool operator!=(const vec2 &v) const { return !(*this == v); }

        inline bool operator==(float s) const { return x == s && y == s; }

        inline bool operator!=(float s) const { return !(*this == s); }

        inline bool operator<(const vec2 &v) const { return x < v.x && y < v.y; }

        inline bool operator>(const vec2 &v) const { return x > v.x && y > v.y; }

        inline vec2 operator-() const { return vec2(-x, -y); }

        vec2 &operator+=(const vec2 &v) {
            x += v.x;
            y += v.y;
            return *this;
        }

        vec2 &operator-=(const vec2 &v) {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        vec2 &operator*=(const vec2 &v) {
            x *= v.x;
            y *= v.y;
            return *this;
        }

        vec2 &operator/=(const vec2 &v) {
            x /= v.x;
            y /= v.y;
            return *this;
        }

        vec2 &operator+=(float s) {
            x += s;
            y += s;
            return *this;
        }

        vec2 &operator-=(float s) {
            x -= s;
            y -= s;
            return *this;
        }

        vec2 &operator*=(float s) {
            x *= s;
            y *= s;
            return *this;
        }

        vec2 &operator/=(float s) {
            x /= s;
            y /= s;
            return *this;
        }

        vec2 operator+(const vec2 &v) const { return vec2(x + v.x, y + v.y); }

        vec2 operator-(const vec2 &v) const { return vec2(x - v.x, y - v.y); }

        vec2 operator*(const vec2 &v) const { return vec2(x * v.x, y * v.y); }

        vec2 operator/(const vec2 &v) const { return vec2(x / v.x, y / v.y); }

        vec2 operator+(float s) const { return vec2(x + s, y + s); }

        vec2 operator-(float s) const { return vec2(x - s, y - s); }

        vec2 operator*(float s) const { return vec2(x * s, y * s); }

        vec2 operator/(float s) const { return vec2(x / s, y / s); }

        float dot(const vec2 &v) const { return x * v.x + y * v.y; }

        float cross(const vec2 &v) const { return x * v.y - y * v.x; }

        float length2() const { return dot(*this); }

        float length() const { return sqrtf(length2()); }

        vec2 abs() const { return vec2(fabsf(x), fabsf(y)); }

        vec2 normal() const {
            float s = length();
            return s == 0.0 ? (*this) : (*this) * (1.0f / s);
        }

        float angle() const { return atan2f(y, x); }

        vec2 &rotate(const vec2 &cs) {
            *this = vec2(x * cs.x - y * cs.y, x * cs.y + y * cs.x);
            return *this;
        }

        vec2 &rotate(float angle) {
            vec2 cs;
            sincos(angle, &cs.y, &cs.x);
            return rotate(cs);
        }
    };

    struct vec3 {
        float x, y, z;

        vec3() {}

        vec3(float s) : x(s), y(s), z(s) {}

        vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        vec3(const vec2 &xy, float z = 0.0f) : x(xy.x), y(xy.y), z(z) {}

        vec3(float lng, float lat) : x(sinf(lat) * cosf(lng)), y(-sinf(lng)), z(cosf(lat) * cosf(lng)) {}

        vec2 &xy() const { return *((vec2 *) &x); }

        vec2 &yz() const { return *((vec2 *) &y); }

        inline float &operator[](int index) const {
            ASSERT(index >= 0 && index <= 2);
            return ((float *) this)[index];
        }

        inline bool operator==(const vec3 &v) const { return x == v.x && y == v.y && z == v.z; }

        inline bool operator!=(const vec3 &v) const { return !(*this == v); }

        inline bool operator==(float s) const { return x == s && y == s && z == s; }

        inline bool operator!=(float s) const { return !(*this == s); }

        inline bool operator<(const vec3 &v) const { return x < v.x && y < v.y && z < v.z; }

        inline bool operator>(const vec3 &v) const { return x > v.x && y > v.y && z > v.z; }

        inline vec3 operator-() const { return vec3(-x, -y, -z); }

        vec3 &operator+=(const vec3 &v) {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        vec3 &operator-=(const vec3 &v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        vec3 &operator*=(const vec3 &v) {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        vec3 &operator/=(const vec3 &v) {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            return *this;
        }

        vec3 &operator+=(float s) {
            x += s;
            y += s;
            z += s;
            return *this;
        }

        vec3 &operator-=(float s) {
            x -= s;
            y -= s;
            z -= s;
            return *this;
        }

        vec3 &operator*=(float s) {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        vec3 &operator/=(float s) {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        vec3 operator+(const vec3 &v) const { return vec3(x + v.x, y + v.y, z + v.z); }

        vec3 operator-(const vec3 &v) const { return vec3(x - v.x, y - v.y, z - v.z); }

        vec3 operator*(const vec3 &v) const { return vec3(x * v.x, y * v.y, z * v.z); }

        vec3 operator/(const vec3 &v) const { return vec3(x / v.x, y / v.y, z / v.z); }

        vec3 operator+(float s) const { return vec3(x + s, y + s, z + s); }

        vec3 operator-(float s) const { return vec3(x - s, y - s, z - s); }

        vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }

        vec3 operator/(float s) const { return vec3(x / s, y / s, z / s); }

        float dot(const vec3 &v) const { return x * v.x + y * v.y + z * v.z; }

        vec3 cross(const vec3 &v) const { return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

        float length2() const { return dot(*this); }

        float length() const { return sqrtf(length2()); }

        vec3 abs() const { return vec3(fabsf(x), fabsf(y), fabsf(z)); }

        vec3 normal() const {
            float s = length();
            return s == 0.0f ? (*this) : (*this) * (1.0f / s);
        }

        vec3 axisXZ() const { return (fabsf(x) > fabsf(z)) ? vec3(float(sign(x)), 0, 0) : vec3(0, 0, float(sign(z))); }

        vec3 reflect(const vec3 &n) const {
            return *this - n * (dot(n) * 2.0f);
        }

        vec3 lerp(const vec3 &v, const float t) const {
            if (t <= 0.0f) return *this;
            if (t >= 1.0f) return v;
            return *this + (v - *this) * t;
        }

        vec3 rotateY(float angle) const {
            float s, c;
            sincos(angle, &s, &c);
            return vec3(x * c - z * s, y, x * s + z * c);
        }

        float angle(const vec3 &v) const {
            return dot(v) / (length() * v.length());
        }

        float angleX() const { return atan2f(sqrtf(x * x + z * z), y); }

        float angleY() const { return atan2f(z, x); }
    };

    struct vec4 {
        float x, y, z, w;

        vec3 &xyz() const { return *((vec3 *) &x); }

        vec4() {}

        vec4(float s) : x(s), y(s), z(s), w(s) {}

        vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        vec4(const vec3 &xyz) : x(xyz.x), y(xyz.y), z(xyz.z), w(0) {}

        vec4(const vec3 &xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

        vec4(const vec2 &xy, const vec2 &zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}

        inline bool operator==(const vec4 &v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }

        inline bool operator!=(const vec4 &v) const { return !(*this == v); }

        vec4 operator+(const vec4 &v) const { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }

        vec4 operator-(const vec4 &v) const { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }

        vec4 operator*(const vec4 &v) const { return vec4(x * v.x, y * v.y, z * v.z, w * v.w); }

        vec4 &operator*=(const vec4 &v) {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            w *= v.w;
            return *this;
        }

        vec4 lerp(const vec4 &v, const float t) const {
            if (t <= 0.0f) return *this;
            if (t >= 1.0f) return v;
            return *this + (v - *this) * t;
        }
    };

    struct quat {
        float x, y, z, w;

        vec3 &xyz() const { return *((vec3 *) &x); }

        quat() {}

        quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        quat(const vec3 &axis, float angle) {
            float s, c;
            sincos(angle * 0.5f, &s, &c);
            x = axis.x * s;
            y = axis.y * s;
            z = axis.z * s;
            w = c;
        }

        quat(const vec3 &lookAt) {
            vec3 forward = lookAt.normal();
            vec3 rotAxis = vec3(0, 0, 1).cross(forward);
            float dot = vec3(0, 0, 1).dot(forward);

            x = rotAxis.x;
            y = rotAxis.y;
            z = rotAxis.z;
            w = dot+1;

            normalize();
        }

        quat(const vec3 &lookAt, const vec3 &upVector) {
            vec3 up = upVector.normal();
            vec3 forward = lookAt.normal();
            vec3 right = up.cross(forward).normal();
            up = forward.cross(right);

            float e00 = right.x,   e10 = right.y,   e20 = right.z;
            float e01 = up.x,      e11 = up.y,      e21 = up.z;
            float e02 = forward.x, e12 = forward.y, e22 = forward.z;

            float t, s;
            t = 1.0f + e00 + e11 + e22;
            if (t > 0.0001f) {
                s = 0.5f / sqrtf(t);
                x = (e21 - e12) * s;
                y = (e02 - e20) * s;
                z = (e10 - e01) * s;
                w = 0.25f / s;
            } else if (e00 > e11 && e00 > e22) {
                s = 0.5f / sqrtf(1.0f + e00 - e11 - e22);
                x = 0.25f / s;
                y = (e01 + e10) * s;
                z = (e02 + e20) * s;
                w = (e21 - e12) * s;
            } else if (e11 > e22) {
                s = 0.5f / sqrtf(1.0f - e00 + e11 - e22);
                x = (e01 + e10) * s;
                y = 0.25f / s;
                z = (e12 + e21) * s;
                w = (e02 - e20) * s;
            } else {
                s = 0.5f / sqrtf(1.0f - e00 - e11 + e22);
                x = (e02 + e20) * s;
                y = (e12 + e21) * s;
                z = 0.25f / s;
                w = (e10 - e01) * s;
            }
        }

        quat operator-() const {
            return quat(-x, -y, -z, -w);
        }

        quat operator+(const quat &q) const {
            return quat(x + q.x, y + q.y, z + q.z, w + q.w);
        }

        quat operator-(const quat &q) const {
            return quat(x - q.x, y - q.y, z - q.z, w - q.w);
        }

        quat operator*(const float s) const {
            return quat(x * s, y * s, z * s, w * s);
        }

        quat operator*(const quat &q) const {
            return quat(w * q.x + x * q.w + y * q.z - z * q.y,
                        w * q.y + y * q.w + z * q.x - x * q.z,
                        w * q.z + z * q.w + x * q.y - y * q.x,
                        w * q.w - x * q.x - y * q.y - z * q.z);
        }

        vec3 operator*(const vec3 &v) const {
            //return v + xyz.cross(xyz.cross(v) + v * w) * 2.0f;
            return (*this * quat(v.x, v.y, v.z, 0) * inverse()).xyz();
        }

        float dot(const quat &q) const {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }

        float length2() const {
            return dot(*this);
        }

        float length() const {
            return sqrtf(length2());
        }

        void normalize() {
            *this = normal();
        }

        quat normal() const {
            return *this * (1.0f / length());
        }

        quat conjugate() const {
            return quat(-x, -y, -z, w);
        }

        quat inverse() const {
            return conjugate() * (1.0f / length2());
        }

        quat lerp(const quat &q, float t) const {
            if (t <= 0.0f) return *this;
            if (t >= 1.0f) return q;

            return dot(q) < 0 ? (*this - (q + *this) * t) :
                   (*this + (q - *this) * t);
        }

        quat slerp(const quat &q, float t) const {
            if (t <= 0.0f) return *this;
            if (t >= 1.0f) return q;

            quat temp;
            float omega, cosom, sinom, scale0, scale1;

            cosom = dot(q);
            if (cosom < 0.0f) {
                temp = -q;
                cosom = -cosom;
            } else
                temp = q;

            if (1.0f - cosom > EPS) {
                omega = acosf(cosom);
                sinom = 1.0f / sinf(omega);
                scale0 = sinf((1.0f - t) * omega) * sinom;
                scale1 = sinf(t * omega) * sinom;
            } else {
                scale0 = 1.0f - t;
                scale1 = t;
            }

            return *this * scale0 + temp * scale1;
        }
    };

    struct mat4 {
        enum ProjRange {
            PROJ_NEG_POS,
            PROJ_ZERO_POS,
        };

        float e00, e10, e20, e30,
                e01, e11, e21, e31,
                e02, e12, e22, e32,
                e03, e13, e23, e33;

        vec3 &right() const { return *((vec3 *) &e00); }

        vec3 &up() const { return *((vec3 *) &e01); }

        vec3 &forward() const { return *((vec3 *) &e02); }

        vec4 &offset() const { return *((vec4 *) &e03); }

        mat4() {}

        mat4(float e00, float e10, float e20, float e30,
             float e01, float e11, float e21, float e31,
             float e02, float e12, float e22, float e32,
             float e03, float e13, float e23, float e33) :
                e00(e00), e10(e10), e20(e20), e30(e30),
                e01(e01), e11(e11), e21(e21), e31(e31),
                e02(e02), e12(e12), e22(e22), e32(e32),
                e03(e03), e13(e13), e23(e23), e33(e33) {}

        mat4(const quat &rot, const vec3 &pos) {
            setRot(rot);
            setPos(pos);
            e30 = e31 = e32 = 0.0f;
            e33 = 1.0f;
        }

        mat4(ProjRange range, float l, float r, float b, float t, float znear, float zfar) {
            identity();
            e00 = 2.0f / (r - l);
            e11 = 2.0f / (t - b);
            e22 = 2.0f / (znear - zfar);
            e03 = (l + r) / (l - r);
            e13 = (t + b) / (b - t);
            switch (range) {
                case PROJ_NEG_POS :
                    e23 = (zfar + znear) / (znear - zfar);
                    break;
                case PROJ_ZERO_POS :
                    e23 = znear / (znear - zfar);
                    break;
            }
        }

        mat4(ProjRange range, float fov, float aspect, float znear, float zfar) {
            float k = 1.0f / tanf(fov * 0.5f * DEG2RAD);
            identity();
            if (aspect >= 1.0f) {
                e00 = k / aspect;
                e11 = k;
            } else {
                e00 = k;
                e11 = k * aspect;
            }
            e33 = 0.0f;
            e32 = -1.0f;
            switch (range) {
                case PROJ_NEG_POS :
                    e22 = (znear + zfar) / (znear - zfar);
                    e23 = 2.0f * zfar * znear / (znear - zfar);
                    break;
                case PROJ_ZERO_POS :
                    e22 = zfar / (znear - zfar);
                    e23 = znear * e22;
                    break;
            }
        }

        mat4(const vec3 &from, const vec3 &at, const vec3 &up) {
            vec3 r, u, d;
            d = (from - at).normal();
            r = up.cross(d).normal();
            u = d.cross(r);

            this->right() = r;
            this->up() = u;
            this->forward() = d;
            this->offset() = vec4(from, 1.0f);
            e30 = e31 = e32 = 0;
        }

        mat4(const vec4 &reflectPlane) {
            float a = reflectPlane.x,
                    b = reflectPlane.y,
                    c = reflectPlane.z,
                    d = reflectPlane.w;

            right() = vec3(1 - 2 * a * a, -2 * b * a, -2 * c * a);
            up() = vec3(-2 * a * b, 1 - 2 * b * b, -2 * c * b);
            forward() = vec3(-2 * a * c, -2 * b * c, 1 - 2 * c * c);
            offset() = vec4(-2 * a * d, -2 * b * d, -2 * c * d, 1);
            e30 = e31 = e32 = 0;
        }


        void identity() {
            e10 = e20 = e30 = e01 = e21 = e31 = e02 = e12 = e32 = e03 = e13 = e23 = 0.0f;
            e00 = e11 = e22 = e33 = 1.0f;
        }

        mat4 operator*(const mat4 &m) const {
            mat4 r;
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

        vec3 operator*(const vec3 &v) const {
            return vec3(
                    e00 * v.x + e01 * v.y + e02 * v.z + e03,
                    e10 * v.x + e11 * v.y + e12 * v.z + e13,
                    e20 * v.x + e21 * v.y + e22 * v.z + e23);
        }

        vec4 operator*(const vec4 &v) const {
            return vec4(
                    e00 * v.x + e01 * v.y + e02 * v.z + e03 * v.w,
                    e10 * v.x + e11 * v.y + e12 * v.z + e13 * v.w,
                    e20 * v.x + e21 * v.y + e22 * v.z + e23 * v.w,
                    e30 * v.x + e31 * v.y + e32 * v.z + e33 * v.w);
        }

        void translate(const vec3 &offset) {
            mat4 m;
            m.identity();
            m.setPos(offset);
            *this = *this * m;
        };

        void scale(const vec3 &factor) {
            mat4 m;
            m.identity();
            m.e00 = factor.x;
            m.e11 = factor.y;
            m.e22 = factor.z;
            *this = *this * m;
        }

        void rotateX(float angle) {
            mat4 m;
            m.identity();
            float s, c;
            sincos(angle, &s, &c);
            m.e11 = c;
            m.e21 = s;
            m.e12 = -s;
            m.e22 = c;
            *this = *this * m;
        }

        void rotateY(float angle) {
            mat4 m;
            m.identity();
            float s, c;
            sincos(angle, &s, &c);
            m.e00 = c;
            m.e20 = -s;
            m.e02 = s;
            m.e22 = c;
            *this = *this * m;
        }

        void rotateZ(float angle) {
            mat4 m;
            m.identity();
            float s, c;
            sincos(angle, &s, &c);
            m.e00 = c;
            m.e01 = -s;
            m.e10 = s;
            m.e11 = c;
            *this = *this * m;
        }

        void rotateYXZ(const vec3 &angle) {
            float s, c, a, b;

            if (angle.y != 0.0f) {
                sincos(angle.y, &s, &c);

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

            if (angle.x != 0.0f) {
                sincos(angle.x, &s, &c);

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

            if (angle.z != 0.0f) {
                sincos(angle.z, &s, &c);

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

        void lerp(const mat4 &m, float t) {
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

        float det() const {
            return e00 *
                   (e11 * (e22 * e33 - e32 * e23) - e21 * (e12 * e33 - e32 * e13) + e31 * (e12 * e23 - e22 * e13)) -
                   e10 *
                   (e01 * (e22 * e33 - e32 * e23) - e21 * (e02 * e33 - e32 * e03) + e31 * (e02 * e23 - e22 * e03)) +
                   e20 *
                   (e01 * (e12 * e33 - e32 * e13) - e11 * (e02 * e33 - e32 * e03) + e31 * (e02 * e13 - e12 * e03)) -
                   e30 *
                   (e01 * (e12 * e23 - e22 * e13) - e11 * (e02 * e23 - e22 * e03) + e21 * (e02 * e13 - e12 * e03));
        }

        mat4 inverse() const {
            float idet = 1.0f / det();
            mat4 r;
            r.e00 = (e11 * (e22 * e33 - e32 * e23) - e21 * (e12 * e33 - e32 * e13) + e31 * (e12 * e23 - e22 * e13)) *
                    idet;
            r.e01 = -(e01 * (e22 * e33 - e32 * e23) - e21 * (e02 * e33 - e32 * e03) + e31 * (e02 * e23 - e22 * e03)) *
                    idet;
            r.e02 = (e01 * (e12 * e33 - e32 * e13) - e11 * (e02 * e33 - e32 * e03) + e31 * (e02 * e13 - e12 * e03)) *
                    idet;
            r.e03 = -(e01 * (e12 * e23 - e22 * e13) - e11 * (e02 * e23 - e22 * e03) + e21 * (e02 * e13 - e12 * e03)) *
                    idet;
            r.e10 = -(e10 * (e22 * e33 - e32 * e23) - e20 * (e12 * e33 - e32 * e13) + e30 * (e12 * e23 - e22 * e13)) *
                    idet;
            r.e11 = (e00 * (e22 * e33 - e32 * e23) - e20 * (e02 * e33 - e32 * e03) + e30 * (e02 * e23 - e22 * e03)) *
                    idet;
            r.e12 = -(e00 * (e12 * e33 - e32 * e13) - e10 * (e02 * e33 - e32 * e03) + e30 * (e02 * e13 - e12 * e03)) *
                    idet;
            r.e13 = (e00 * (e12 * e23 - e22 * e13) - e10 * (e02 * e23 - e22 * e03) + e20 * (e02 * e13 - e12 * e03)) *
                    idet;
            r.e20 = (e10 * (e21 * e33 - e31 * e23) - e20 * (e11 * e33 - e31 * e13) + e30 * (e11 * e23 - e21 * e13)) *
                    idet;
            r.e21 = -(e00 * (e21 * e33 - e31 * e23) - e20 * (e01 * e33 - e31 * e03) + e30 * (e01 * e23 - e21 * e03)) *
                    idet;
            r.e22 = (e00 * (e11 * e33 - e31 * e13) - e10 * (e01 * e33 - e31 * e03) + e30 * (e01 * e13 - e11 * e03)) *
                    idet;
            r.e23 = -(e00 * (e11 * e23 - e21 * e13) - e10 * (e01 * e23 - e21 * e03) + e20 * (e01 * e13 - e11 * e03)) *
                    idet;
            r.e30 = -(e10 * (e21 * e32 - e31 * e22) - e20 * (e11 * e32 - e31 * e12) + e30 * (e11 * e22 - e21 * e12)) *
                    idet;
            r.e31 = (e00 * (e21 * e32 - e31 * e22) - e20 * (e01 * e32 - e31 * e02) + e30 * (e01 * e22 - e21 * e02)) *
                    idet;
            r.e32 = -(e00 * (e11 * e32 - e31 * e12) - e10 * (e01 * e32 - e31 * e02) + e30 * (e01 * e12 - e11 * e02)) *
                    idet;
            r.e33 = (e00 * (e11 * e22 - e21 * e12) - e10 * (e01 * e22 - e21 * e02) + e20 * (e01 * e12 - e11 * e02)) *
                    idet;
            return r;
        }

        mat4 inverseOrtho() const {
            mat4 r;
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

        mat4 transpose() const {
            mat4 r;
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

        quat getRot() const {
            float t, s;
            t = 1.0f + e00 + e11 + e22;
            if (t > 0.0001f) {
                s = 0.5f / sqrtf(t);
                return quat((e21 - e12) * s, (e02 - e20) * s, (e10 - e01) * s, 0.25f / s);
            } else if (e00 > e11 && e00 > e22) {
                s = 0.5f / sqrtf(1.0f + e00 - e11 - e22);
                return quat(0.25f / s, (e01 + e10) * s, (e02 + e20) * s, (e21 - e12) * s);
            } else if (e11 > e22) {
                s = 0.5f / sqrtf(1.0f - e00 + e11 - e22);
                return quat((e01 + e10) * s, 0.25f / s, (e12 + e21) * s, (e02 - e20) * s);
            } else {
                s = 0.5f / sqrtf(1.0f - e00 - e11 + e22);
                return quat((e02 + e20) * s, (e12 + e21) * s, 0.25f / s, (e10 - e01) * s);
            }
        }

        void setRot(const quat &rot) {
            float   sx = rot.x * rot.x,
                    sy = rot.y * rot.y,
                    sz = rot.z * rot.z,
                    sw = rot.w * rot.w,
                    inv = 1.0f / (sx + sy + sz + sw);

            e00 = ( sx - sy - sz + sw) * inv;
            e11 = (-sx + sy - sz + sw) * inv;
            e22 = (-sx - sy + sz + sw) * inv;
            inv *= 2.0f;

            float t1 = rot.x * rot.y;
            float t2 = rot.z * rot.w;
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

        vec3 getPos() const {
            return offset().xyz();
        }

        void setPos(const vec3 &pos) {
            offset().xyz() = pos;
        }
    };

    struct Basis {
        quat rot;
        vec3 pos;
        float w;

        Basis() {}

        Basis(const quat &rot, const vec3 &pos) : rot(rot), pos(pos), w(1.0f) {}

        Basis(const mat4 &matrix) : rot(matrix.getRot()), pos(matrix.getPos()), w(1.0f) {}

        void identity() {
            rot = quat(0, 0, 0, 1);
            pos = vec3(0, 0, 0);
            w = 1.0f;
        }

        Basis operator*(const Basis &basis) const {
            return Basis(rot * basis.rot, pos + rot * basis.pos);
        }

        vec3 operator*(const vec3 &v) const {
            return rot * v + pos;
        }

        Basis inverse() const {
            quat q = rot.conjugate();
            return Basis(q, -(q * pos));
        }

        void translate(const vec3 &v) {
            pos += rot * v;
        }

        void rotate(const quat &q) {
            rot = rot * q;
        }

        Basis lerp(const Basis &basis, float t) {
            if (t <= 0.0f) return *this;
            if (t >= 1.0f) return basis;
            Basis b;
            b.rot = rot.lerp(basis.rot, t);
            b.pos = pos.lerp(basis.pos, t);
            return b;
        }
    };
}