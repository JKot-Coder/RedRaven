#pragma once

#include "Transform.hpp"

namespace OpenDemo
{
    namespace Rendering
    {

        struct Transform;

        class Camera
        {
        public:
            struct Description
            {
                bool isOrtho;
                float aspect;
                float fow;
                float orthoSize;
                float zNear;
                float zFar;
            };

            Camera(const Description& description)
                : _isOrtho(description.isOrtho)
                , _aspect(description.aspect)
                , _fov(description.fow)
                , _orthoSize(description.orthoSize)
                , _zNear(description.zNear)
                , _zFar(description.zFar)
                , _transform()
            {
                calcProjectionMatrix();
            }

            inline void SetOrtho(bool value)
            {
                if (_isOrtho == value)
                {
                    return;
                }

                _isOrtho = value;
                calcProjectionMatrix();
            }

            inline void SetAspect(int width, int height)
            {
                SetAspect(static_cast<float>(width) / static_cast<float>(height));
            }

            inline void SetAspect(float value)
            {
                if (_aspect == value)
                {
                    return;
                }

                _aspect = value;
                calcProjectionMatrix();
            }

            inline void SetFov(float value)
            {
                if (_fov == value)
                {
                    return;
                }

                _fov = value;
                calcProjectionMatrix();
            }

            inline void SetOrthoSize(float value)
            {
                if (_orthoSize == value)
                {
                    return;
                }

                _orthoSize = value;
                calcProjectionMatrix();
            }

            inline void SetZField(float zNear_, float zFar_)
            {
                if (_zNear == zNear_ && _zFar == zFar_)
                {
                    return;
                }

                _zNear = zNear_;
                _zFar = zFar_;
                calcProjectionMatrix();
            }

            inline void SetTransform(const Transform& value) { _transform = value; }
            inline Transform GetTransform() const { return _transform; }

            inline mat4 GetViewMatrix() const { return _transform.GetMatrix(); }
            inline mat4 GetViewProjectionMatrix() const { return GetProjectionMatrix() * GetViewMatrix().inverseOrtho(); }

            inline mat4 GetProjectionMatrix() const { return _projectionMatrix; }

            inline void LookAt(vec3 eyePosition, vec3 targetPosition)
            {
                _transform.Position = eyePosition;
                _transform.Rotation = quat(targetPosition - eyePosition);
            }

        private:
            bool _isOrtho;
            float _aspect;
            float _fov;
            float _orthoSize;
            float _zNear;
            float _zFar;

            Transform _transform;
            mat4 _projectionMatrix;

            inline void calcProjectionMatrix()
            {
                if (_isOrtho)
                {
                    const float width = _orthoSize * _aspect;
                    const float height = _orthoSize;

                    _projectionMatrix = mat4(mat4::PROJ_ZERO_POS, -width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, _zNear, _zFar);
                }
                else
                {
                    _projectionMatrix = mat4(mat4::PROJ_ZERO_POS, _fov, _aspect, _zNear, _zFar);
                }
            };
        };

    }
}