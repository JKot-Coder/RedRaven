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
            zFar(description.zFar)
        {
            calcProjectionMatrix();
        }

        inline void SetOrtho(bool isOrtho) {
            if (isOrtho == this->isOrtho){
                return;
            }

            this->isOrtho = isOrtho;
            calcProjectionMatrix();
        }

        inline void SetAspect(float width, float height){
            SetAspect(width/height);
        }

        inline void SetAspect(float aspect) {
            if (aspect == this->aspect){
                return;
            }

            this->aspect = aspect;
            calcProjectionMatrix();
        }

        inline void SetFov(float fov){
            if (fov == this->fov){
                return;
            }

            this->fov = fov;
            calcProjectionMatrix();
        }

        inline void SetOrthoSize(float orthoSize) {
            if (orthoSize == this->orthoSize){
                return;
            }

            this->orthoSize = orthoSize;
            calcProjectionMatrix();
        }

        inline void SetZField(float zNear, float zFar) {
            if (zNear == this->zNear && zFar == this->zFar){
                return;
            }

            this->zNear = zNear;
            this->zFar = zFar;
            calcProjectionMatrix();
        }

        inline Transform GetTransform() const { return transform; }

        inline mat4 GetViewMatrix() const { return transform.GetMatrix(); }
        inline mat4 GetViewProjectionMatrix() const {
            return GetProjectionMatrix() * GetViewMatrix();
        }

        inline mat4 GetProjectionMatrix() const {
            return projectionMatrix;
        }

        inline void LookAt(vec3 eyePosition, vec3 targetPosition) {
            transform.SetPostion(eyePosition);
            transform.SetRotation(quat(targetPosition - eyePosition, vec3(0, 1, 0)));
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

                projectionMatrix = mat4(mat4::PROJ_ZERO_POS, -width * 0.5, width * 0.5, -height * 0.5, height * 0.5 );
            } else {
                projectionMatrix = mat4(mat4::PROJ_ZERO_POS, fov, aspect, zNear, zFar);
            }
        };

    };

}
