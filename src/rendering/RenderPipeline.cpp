#include "windowing/Window.hpp"

#include "rendering/Texture.hpp"
#include "rendering/RenderTarget.hpp"
#include "rendering/RenderTargetContext.hpp"

#include "rendering/Render.hpp"

#include "rendering/RenderPipeline.hpp"
#include "RenderPipeline.hpp"

namespace Rendering{

    RenderPipeline::RenderPipeline(const std::shared_ptr<Windowing::Window> &window) : window(window) {
        Windowing::Windowing::Subscribe(this);
    }

    RenderPipeline::~RenderPipeline() {
        Windowing::Windowing::UnSubscribe(this);
    }

    void RenderPipeline::Init() {
        const auto &render = Render::Instance();

        Texture2D::Description textureDescription;
        textureDescription.height = window->GetHeight();
        textureDescription.width  = window->GetWidth();
        textureDescription.pixelFormat = PixelFormat::RGBA32F;

        auto const &hdrTexture = render->CreateTexture2D();
        hdrTexture->Init(textureDescription, nullptr);

        textureDescription.pixelFormat = PixelFormat::D16;
        auto const &depthTexture = render->CreateTexture2D();
        depthTexture->Init(textureDescription, nullptr);

        RenderTarget::RenderTargetDescription colorTarget;
        colorTarget.texture = hdrTexture;

        RenderTarget::RenderTargetDescription depthTarget;
        depthTarget.texture = depthTexture;
        depthTarget.isDepthTarget = true;

        hdrRenderTargetContext = render->CreateRenderTargetContext();
        hdrRenderTargetContext->SetColorTarget(RenderTargetIndex::INDEX_0, colorTarget);
        hdrRenderTargetContext->SetDepthStencilTarget(depthTarget);

        hdrRenderTargetContext->Bind();
        render->ClearColor(vec4(0,0,0,0));

        initPass<RenderPassOpaque>(*render, hdrRenderTargetContext);
        initPass<RenderPassPostProcess>(*render, hdrTexture);
    }

    void RenderPipeline::Collect(const std::shared_ptr<SceneGraph> &sceneGraph) {
        getPass<RenderPassOpaque>()->Collect(sceneGraph);
        getPass<RenderPassPostProcess>()->Collect(sceneGraph);
    }

    void RenderPipeline::Draw() {
        getPass<RenderPassOpaque>()->Draw();
        getPass<RenderPassPostProcess>()->Draw();
    }

    void RenderPipeline::WindowResize(const Windowing::Window &window) {
        int width = window.GetWidth();
        int height = window.GetHeight();
        hdrRenderTargetContext->Resize(width, height);
    }

}