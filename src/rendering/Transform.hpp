#pragma once

#include "common/VecMath.h"

using namespace Common;

namespace Rendering {

    struct Transform {
    public:

        inline void Identity() { position = vec3(0,0,0); rotation = quat(0, 0, 0, 1); }

        inline void SetPostion(const vec3 &position) { this->position = position; }
        inline void SetRotation(const quat &rotation) { this->rotation = rotation; }

        inline mat4 GetMatrix() const { return mat4(rotation, position); }
        inline vec3 GetPostion() const { return position; }
        inline quat GetRotation() const { return rotation; }

    private:
        vec3 position;
        quat rotation;
    };

}