#pragma once

#include "common/VecMath.h"

using namespace Common;

namespace Rendering {

    struct Transform {
    public:
        vec3 Position;
        quat Rotation;

        inline void Identity() { Position = vec3(0,0,0); Rotation = quat(0, 0, 0, 1); }
        inline mat4 GetMatrix() const { return mat4(Rotation, Position); }
    };

}