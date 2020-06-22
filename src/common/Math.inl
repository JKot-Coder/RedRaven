namespace OpenDemo
{
    namespace Common
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Vector2
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        INLINE T& Vector2<T>::operator[](int index) const
        {
            ASSERT(index >= 0 && index <= 1)
            return ((T*)this)[index];
        }

        template <typename T>
        INLINE bool Vector2<T>::operator==(const Vector2<T>& v) const { return x == v.x && y == v.y; }

        template <typename T>
        INLINE bool Vector2<T>::operator!=(const Vector2<T>& v) const { return !(*this == v); }

        template <typename T>
        INLINE bool Vector2<T>::operator==(T s) const { return x == s && y == s; }

        template <typename T>
        INLINE bool Vector2<T>::operator!=(T s) const { return !(*this == s); }

        template <typename T>
        INLINE bool Vector2<T>::operator<(const Vector2<T>& v) const { return x < v.x && y < v.y; }

        template <typename T>
        INLINE bool Vector2<T>::operator>(const Vector2<T>& v) const { return x > v.x && y > v.y; }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator-() const { return Vector2<T>(-x, -y); }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        template <typename T>
        Vector2<T>& Vector2<T>::operator*=(const Vector2<T>& v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator/=(const Vector2<T>& v)
        {
            x /= v.x;
            y /= v.y;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator+=(T s)
        {
            x += s;
            y += s;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator-=(T s)
        {
            x -= s;
            y -= s;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator*=(T s)
        {
            x *= s;
            y *= s;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::operator/=(T s)
        {
            x /= s;
            y /= s;
            return *this;
        }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator+(const Vector2<T>& v) const { return Vector2<T>(x + v.x, y + v.y); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator-(const Vector2<T>& v) const { return Vector2<T>(x - v.x, y - v.y); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator*(const Vector2<T>& v) const { return Vector2<T>(x * v.x, y * v.y); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator/(const Vector2<T>& v) const { return Vector2<T>(x / v.x, y / v.y); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator+(T s) const { return Vector2<T>(x + s, y + s); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator-(T s) const { return Vector2<T>(x - s, y - s); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator*(T s) const { return Vector2<T>(x * s, y * s); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::operator/(T s) const { return Vector2<T>(x / s, y / s); }

        template <typename T>
        INLINE T Vector2<T>::dot(const Vector2<T>& v) const { return x * v.x + y * v.y; }

        template <typename T>
        INLINE T Vector2<T>::cross(const Vector2<T>& v) const { return x * v.y - y * v.x; }

        template <typename T>
        INLINE T Vector2<T>::length2() const { return dot(*this); }

        template <typename T>
        INLINE T Vector2<T>::length() const { return sqrtf(length2()); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::abs() const { return Vector2<T>(fabsf(x), fabsf(y)); }

        template <typename T>
        INLINE Vector2<T> Vector2<T>::normal() const
        {
            T s = length();
            return s == 0.0 ? (*this) : (*this) * (1.0f / s);
        }

        template <typename T>
        INLINE T Vector2<T>::angle() const { return atan2f(y, x); }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::rotate(const Vector2<T>& cs)
        {
            *this = Vector2<T>(x * cs.x - y * cs.y, x * cs.y + y * cs.x);
            return *this;
        }

        template <typename T>
        INLINE Vector2<T>& Vector2<T>::rotate(T angle)
        {
            Vector2<T> cs;
            sincos(angle, &cs.y, &cs.x);
            return rotate(cs);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        // Rect
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        INLINE bool Rect<T>::contains(T x, T y) const
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
        INLINE bool Rect<T>::contains(const Vector2<T>& point) const
        {
            return contains(point.x, point.y);
        }

        template <typename T>
        INLINE bool Rect<T>::intersects(const Rect<T>& rect) const
        {
            Rect<T> intersection;
            return intersects(rect, intersection);
        }

        template <typename T>
        INLINE bool Rect<T>::intersects(const Rect<T>& rect, Rect<T>& intersection) const
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
        INLINE Vector2<T> Rect<T>::getPosition() const
        {
            return Vector2<T>(left, top);
        }

        template <typename T>
        INLINE Vector2<T> Rect<T>::getSize() const
        {
            return Vector2<T>(width, height);
        }

        template <typename T>
        inline bool Rect<T>::operator==(const Rect<T>& rect) const
        {
            return (left == rect.left) && (width == rect.width) && (top == rect.top) && (height == rect.height);
        }

        template <typename T>
        inline bool Rect<T>::operator!=(const Rect<T>& rect) const
        {
            return !(left == rect);
        }

    }
}
