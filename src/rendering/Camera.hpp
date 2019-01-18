#pragma once

#include "Transform.hpp"

namespace Rendering {

    struct Transform;

    class Camera {
    public:

        struct Description {
            bool isOrtho;
            float aspect;
            float fow;
            float orthoSize;
            float zNear;
            float zFar;
        };

        Camera(const Description &description) :
            isOrtho(description.isOrtho),
            aspect(description.aspect),
            fov(description.fow),
            orthoSize(description.orthoSize),
            zNear(description.zNear),
            zFar(description.zFar),
            transform()
        {
            calcProjectionMatrix();
        }

        inline void SetOrtho(bool value) {
            if (isOrtho == value){
                return;
            }

            isOrtho = value;
            calcProjectionMatrix();
        }

        inline void SetAspect(int width, int height) {
            SetAspect(static_cast<float>(width) / static_cast<float>(height));
        }

        inline void SetAspect(float value) {
            if (aspect == value){
                return;
            }

            aspect = value;
            calcProjectionMatrix();
        }

        inline void SetFov(float value) {
            if (fov == value){
                return;
            }

            fov = value;
            calcProjectionMatrix();
        }

        inline void SetOrthoSize(float value) {
            if (orthoSize == value){
                return;
            }

            orthoSize = value;
            calcProjectionMatrix();
        }

        inline void SetZField(float zNear_, float zFar_) {
            if (zNear == zNear_ && zFar == zFar_){
                return;
            }

            zNear = zNear_;
            zFar = zFar_;
            calcProjectionMatrix();
        }

        inline void SetTransform(const Transform &value) { transform = value; }
        inline Transform GetTransform() const { return transform; }

        inline mat4 GetViewMatrix() const { return transform.GetMatrix(); }
        inline mat4 GetViewProjectionMatrix() const { return GetProjectionMatrix() * GetViewMatrix().inverseOrtho(); }

        inline mat4 GetProjectionMatrix() const { return projectionMatrix; }

        inline void LookAt(vec3 eyePosition, vec3 targetPosition) {
            transform.Position = eyePosition;
            transform.Rotation = quat(targetPosition - eyePosition);
        }

    private:
        bool isOrtho;
        float aspect;
        float fov;
        float orthoSize;
        float zNear;
        float zFar;

        Transform transform;
        mat4 projectionMatrix;

        inline void calcProjectionMatrix() {
            if (isOrtho) {
                const float width  = orthoSize * aspect;
                const float height = orthoSize;

                projectionMatrix = mat4(mat4::PROJ_ZERO_POS, -width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
            } else {
                projectionMatrix = mat4(mat4::PROJ_ZERO_POS, fov, aspect, zNear, zFar);
            }
        };

    };

}
