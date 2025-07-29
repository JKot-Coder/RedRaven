#pragma once

#include "math/Math.hpp"


struct Vector
{
    template <typename T>
    inline const Vector<Vector<2, T>::SIZE, T> Vector<2, T>::ZERO = Vector<2, T>(0);
};
