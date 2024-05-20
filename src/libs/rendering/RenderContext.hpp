#pragma once

#include "math/Math.hpp"

#include "rendering/BlendingDescription.hpp"
#include "rendering/DepthDescription.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace Rendering
    {
        class Mesh;
        class Shader;
        class Camera;
        class RenderTargetContext;
        struct RenderElement;
        enum DepthTestFunction : int;

        typedef std::vector<RenderElement> RenderQuery;

        class RenderContext
        {
        public:
            RenderContext();

            inline void SetDepthWrite(bool value) { _depthWrite = value; }
            inline void SetDepthTestFunction(DepthTestFunction value) { _depthTestFunction = value; }

            inline void SetBlending(bool value) { _blending = value; }
            inline void SetBlendingDescription(const BlendingDescription& value) { _blendingDescription = value; }

            inline void SetCamera(const std::shared_ptr<Camera>& value) { _camera = value; }
            inline void SetRenderTarget(const std::shared_ptr<RenderTargetContext>& value) { _renderTargetContext = value; }
            inline void SetShader(const std::shared_ptr<Shader>& value) { _shader = value; }
            inline void SetLightDirection(const Vector3& value) { _lightDirection = value; }

            inline bool GetDepthWrite() const { return _depthWrite; }
            inline DepthTestFunction GetDepthTestFunction() const { return _depthTestFunction; }

            inline bool GetBlending() const { return _blending; }
            inline BlendingDescription GetBlendingDescription() const { return _blendingDescription; }

            inline std::shared_ptr<Camera> GetCamera() const { return _camera; }
            inline std::shared_ptr<RenderTargetContext> GetRenderTarget() const { return _renderTargetContext; }
            inline std::shared_ptr<Shader> GetShader() const { return _shader; }
            inline RenderQuery& GetRenderQuery() const { return *_renderQuery; }
            inline Vector3 GetLightDirection() const { return _lightDirection; }

        private:
            bool _depthWrite = true;
            DepthTestFunction _depthTestFunction = DepthTestFunction::LEQUAL;

            bool _blending = false;
            BlendingDescription _blendingDescription;

            Vector3 _lightDirection;
            std::unique_ptr<RenderQuery> _renderQuery;
            std::shared_ptr<Camera> _camera;
            std::shared_ptr<RenderTargetContext> _renderTargetContext;
            std::shared_ptr<Shader> _shader;
        };
    }
}