#pragma once

#include <memory>
#include <vector>

#include "common/VecMath.h"

using namespace Common;

namespace Rendering {

    class Mesh;
    class Shader;
    class Material;
    class Camera;
    class RenderTargetContext;
    enum DepthTestFunction : int;

    struct RenderElement {
    public:
        mat4 modelMatrix;
        std::shared_ptr<Rendering::Material> material;
        std::shared_ptr<Rendering::Mesh> mesh;
    };

    typedef std::vector<RenderElement> RenderQuery;

    class RenderContext {
    public:
        RenderContext();

        inline void SetDepthWrite(bool value) { this->depthWrite = value; }
        inline void SetDepthTestFunction(DepthTestFunction function) { this->depthTestFunction = function; }

        inline void SetCamera(const std::shared_ptr<Camera> &camera) { this->camera = camera; }
        inline void SetRenderTarget(const std::shared_ptr<RenderTargetContext> &renderTargetContext) { this->renderTargetContext = renderTargetContext; }
        inline void SetShader(const std::shared_ptr<Shader> &shader) { this->shader = shader; }

        inline bool GetDepthWrite() const { return depthWrite; }
        inline DepthTestFunction GetDepthTestFunction() const { return depthTestFunction; }

        inline std::shared_ptr<Camera> GetCamera() const { return camera; }
        inline std::shared_ptr<RenderTargetContext> GetRenderTarget() const { return renderTargetContext; }
        inline std::shared_ptr<Shader> GetShader() const { return shader; }
        inline RenderQuery& GetRenderQuery() const { return *renderQuery; }

    private:
        bool depthWrite;
        DepthTestFunction depthTestFunction;

        std::unique_ptr<RenderQuery> renderQuery;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<RenderTargetContext> renderTargetContext;
        std::shared_ptr<Shader> shader;
    };

}