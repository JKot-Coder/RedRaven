#pragma once

#include <tuple>
#include <vector>
#include <memory>

#include "rendering/RenderContext.hpp"

namespace Rendering {

    class Render;
    class SceneGraph;
    class Shader;
    class Texture2D;
    class RenderContext;
    class Mesh;

    enum class RenderPassType {
        OPAQUE,
        POST_PROCESS
    };

    class RenderPass {
    public:
        virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) = 0;
        virtual void Draw() = 0;
    };

    class RenderPassOpaque final: public RenderPass {
    public:
        RenderPassOpaque(Rendering::Render& render, const std::shared_ptr<RenderTargetContext> &hdrRenderTargetContext);

        virtual void Collect(const std::shared_ptr<SceneGraph> &sceneGraph) override;
        virtual void Draw() override;

    private:
        Render* render;
        std::shared_ptr<RenderTargetContext> hdrRenderTargetContext;
        std::shared_ptr<RenderContext> renderContext;
        std::shared_ptr<Shader> pbrShader;
    };

    class RenderPassPostProcess final: public RenderPass {
    public:
        RenderPassPostProcess(Rendering::Render& render, const std::shared_ptr<Texture2D> &hdrTexture);

        virtual void Collect(const std::shared_ptr<SceneGraph> &sceneGraph) override;
        virtual void Draw() override;

    private:
        Render* render;
        std::shared_ptr<Texture2D> hdrTexture;
        std::shared_ptr<RenderContext> renderContext;
        std::shared_ptr<Shader> postProcessShader;
        std::shared_ptr<Mesh> fullScreenQuad;
    };

}


