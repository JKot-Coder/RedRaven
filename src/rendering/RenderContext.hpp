#pragma once

#include <memory>
#include <vector>

#include "common/VecMath.h"

#include "rendering/Material.hpp"
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

        inline void SetDepthWrite(bool value) { this->depthWrite = value; }
        inline void SetDepthTestFunction(DepthTestFunction function) { this->depthTestFunction = function; }

        inline void SetBlending(bool value) { this->blending = value; }
        inline void SetBlendingDescription(const BlendingDescription& blendingDescription) { this->blendingDescription = blendingDescription; }

        inline void SetCamera(const std::shared_ptr<Camera> &camera) { this->camera = camera; }
        inline void SetRenderTarget(const std::shared_ptr<RenderTargetContext> &renderTargetContext) { this->renderTargetContext = renderTargetContext; }
        inline void SetShader(const std::shared_ptr<Shader> &shader) { this->shader = shader; }
        inline void SetLightDirection(const vec3& direction) { this->lightDirection = direction; }

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