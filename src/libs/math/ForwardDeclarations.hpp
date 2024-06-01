#pragma once

namespace RR
{
    template <size_t Len, typename T>
    struct Vector;

    using Vector2 = Vector<2, float>;
    using Vector2i = Vector<2, int32_t>;

    using Vector3 = Vector<3, float>;
    using Vector3u = Vector<3, uint32_t>;

    using Vector4 = Vector<4, float>;
    using Vector4u = Vector<4, uint32_t>;

    template <size_t M, size_t K>
    struct Matrix;

    using Matrix4 = Matrix<4, 4>;

    template <typename T>
    struct Rect;

    using Rect2i = Rect<int32_t>;

    template <typename T>
    struct Box;

    using Box3u = Box<uint32_t>;
}