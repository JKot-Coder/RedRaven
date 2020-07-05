namespace OpenDemo
{
    namespace Common
    {

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector2
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<2, T>
        {
            static inline constexpr size_t SIZE = 2;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            Vector() = default;

            Vector(T s)
                : x(s)
                , y(s)
            {
            }

            Vector(T x, T y)
                : x(x)
                , y(y)
            {
            }

            template <typename U>
            Vector(const Vector<SIZE, U>& vector)
                : x(static_cast<T>(vector.x))
                , y(static_cast<T>(vector.y))
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

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector3
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<3, T>
        {
            static inline constexpr size_t SIZE = 3;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            Vector() = default;

            Vector(T s)
                : x(s)
                , y(s)
                , z(s)
            {
            }

            Vector(T x, T y, T z)
                : x(x)
                , y(y)
                , z(z)
            {
            }

            template <typename U>
            Vector(const Vector<SIZE, U>& vector)
                : x(static_cast<T>(vector.x))
                , y(static_cast<T>(vector.y))
                , z(static_cast<T>(vector.z))
            {
            }

            Vector(const Vector<SIZE - 1, T>& xy, float z)
                : x(xy.x)
                , y(xy.y)
                , z(z)
            {
            }

            Vector(float lng, float lat)
                : x(sinf(lat) * cosf(lng))
                , y(-sinf(lng))
                , z(cosf(lat) * cosf(lng))
            {
            }

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

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            const Vector<SIZE, T> RotateY(float angle) const
            {
                float s, c;
                sincos(angle, &s, &c);
                return vec3(x * c - z * s, y, x * s + z * c);
            }

            float Angle(const Vector<SIZE, T>& v) const
            {
                return Dot(v) / (Length() * v.Length());
            }

            float AngleX() const { return atan2f(sqrtf(x * x + z * z), y); }

            float AngleY() const { return atan2f(z, x); }

            // Shared vectors functions
            T& operator[](int index) const;

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
            T z;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector4
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        struct Vector<4, T>
        {
            static inline constexpr size_t SIZE = 4;
            using FloatType = typename std::conditional<std::is_same<T, double>::value, double, float>::type;

            Vector() = default;

            Vector(T s)
                : x(s)
                , y(s)
                , z(s)
                , w(s)
            {
            }

            Vector(T x, T y, T z, T w)
                : x(x)
                , y(y)
                , z(z)
                , w(w)
            {
            }

            template <typename U>
            Vector(const Vector<SIZE, U>& vector)
                : x(static_cast<T>(vector.x))
                , y(static_cast<T>(vector.y))
                , z(static_cast<T>(vector.z))
                , w(static_cast<T>(vector.w))
            {
            }

            Vector(const vec3& xyz, float w)
                : x(xyz.x)
                , y(xyz.y)
                , z(xyz.z)
                , w(w)
            {
            }

            Vector(const Vector<SIZE - 2, T>& xy, const Vector<SIZE - 2, T>& zw)
                : x(xy.x)
                , y(xy.y)
                , z(zw.x)
                , w(zw.y)
            {
            }

            Vector<SIZE - 1, T>& xy() const { return *((Vector<SIZE - 1, T>*)&x); }
            Vector<SIZE - 1, T>& yz() const { return *((Vector<SIZE - 1, T>*)&y); }

            bool operator==(const Vector<SIZE, T>& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const Vector<SIZE, T>& v) const { return !(*this == v); }
            bool operator==(T s) const { return x == s && y == s && z == s && w == s; }
            bool operator!=(T s) const { return !(*this == s); }
            bool operator<(const Vector<SIZE, T>& v) const { return x < v.x && y < v.y && z < v.z && w < v.w; }
            bool operator>(const Vector<SIZE, T>& v) const { return x > v.x && y > v.y && z > v.z && w > v.w; }

            Vector<SIZE, T> operator-() const { return Vector<SIZE, T>(-x, -y, -z); }

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
            Vector<SIZE, T> Cross(const Vector<SIZE, T>& v) const { return Vector<SIZE, T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
            Vector<SIZE, T> Abs() const { return Vector<SIZE, T>(fabsf(x), fabsf(y), fabsf(z)); }
            Vector<SIZE, T> AxisXZ() const { return (fabsf(x) > fabsf(z)) ? Vector<SIZE, T>(float(Sign(x)), 0, 0) : Vector<SIZE, T>(0, 0, float(Sign(z))); }

            Vector<SIZE, T> Reflect(const Vector<SIZE, T>& n) const
            {
                return *this - n * (Dot(n) * 2.0f);
            }

            const Vector<SIZE, T> Lerp(const Vector<SIZE, T>& v, const float t) const
            {
                if (t <= 0.0f)
                    return *this;
                if (t >= 1.0f)
                    return v;
                return *this + (v - *this) * t;
            }

            const Vector<SIZE, T> RotateY(float angle) const
            {
                float s, c;
                sincos(angle, &s, &c);
                return vec3(x * c - z * s, y, x * s + z * c);
            }

            float Angle(const Vector<SIZE, T>& v) const
            {
                return Dot(v) / (Length() * v.Length());
            }

            float AngleX() const { return atan2f(sqrtf(x * x + z * z), y); }

            float AngleY() const { return atan2f(z, x); }

            // Shared vectors functions
            T& operator[](int index) const;

            FloatType Length2() const;
            FloatType Length() const;

            template <typename = std::enable_if<std::is_floating_point<T>::value>::type>
            Vector<SIZE, T> Normal() const;
            ////////////////////////////////

            T x;
            T y;
            T z;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Rect
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

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

    }
}
