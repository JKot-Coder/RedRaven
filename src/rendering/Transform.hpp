#pragma once

#include "common/Math.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace Rendering
    {

        struct Transform
        {
        public:
            Vector3 Position;
            Quaternion Rotation;

            Transform()
            {
                Identity();
            }

            inline void Identity()
            {
                Position = Vector3(0, 0, 0);
                Rotation = Quaternion(1, 0, 0, 0);
            }

            inline Matrix4 GetMatrix() const
            {
                Matrix4 result;

                result.setIdentity();
                result.block<3, 3>(0, 0) = Rotation.toRotationMatrix();
                result.block<3, 1>(0, 3) = Position;

                return result;
            }
        };
    }
}