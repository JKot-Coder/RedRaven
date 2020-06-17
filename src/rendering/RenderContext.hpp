#pragma once

#include <memory>
#include <vector>

#include "common/VecMath.h"

#include "rendering/BlendingDescription.hpp"
#include "rendering/DepthDescription.hpp"

using namespace Common;

namespace Rendering {

    class Mesh;
    class Shader;
    class Camera;
    class RenderTargetContext;
    struct RenderElement;
    enum DepthTestFunction : int;

    typedef std::vector<RenderElement> RenderQuery;

    class RenderContext {
    public:
        RenderContext();

        inline void SetDepthWrite(bool value) { depthWrite = value; }
        inline void SetDepthTestFunction(DepthTestFunction value) { depthTestFunction = value; }

        inline void SetBlending(bool value) { blending = value; }
        inline void SetBlendingDescription(const BlendingDescription& value) { blendingDescription = value; }

        inline void SetCamera(const std::shared_ptr<Camera> &value) { camera = value; }
        inline void SetRenderTarget(const std::shared_ptr<RenderTargetContext> &value) { renderTargetContext = value; }
        inline void SetShader(const std::shared_ptr<Shader> &value) { shader = value; }
        inline void SetLightDirection(const vec3& value) { lightDirection = value; }

        inline bool GetDepthWrite() const { return depthWrite; }
        inline DepthTestFunction GetDepthTestFunction() const { return depthTestFunction; }

        inline bool GetBlending() const { return blending; }
        inline BlendingDescription GetBlendingDescription() const { return blendingDescription; }

        inline std::shared_ptr<Camera> GetCamera() const { return camera; }
        inline std::shared_ptr<RenderTargetContext> GetRenderTarget() const { return renderTargetContext; }
        inline std::shared_ptr<Shader> GetShader() const { return shader; }
        inline RenderQuery& GetRenderQuery() const { return *renderQuery; }
        inline vec3 GetLightDirection() const { return lightDirection; }

    private:
        bool depthWrite = true;
        DepthTestFunction depthTestFunction = DepthTestFunction::LEQUAL;

        bool blending = false;
        BlendingDescription blendingDescription;

        vec3 lightDirection;
        std::unique_ptr<RenderQuery> renderQuery;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<RenderTargetContext> renderTargetContext;
        std::shared_ptr<Shader> shader;
    };

}