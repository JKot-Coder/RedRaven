#include "RenderPipeline.hpp"

#include "windowing/Window.hpp"

#include "rendering/RenderTarget.hpp"
#include "rendering/RenderTargetContext.hpp"
#include "rendering/Texture.hpp"

#include "rendering/Render.hpp"

#include "rendering/RenderPipeline.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        RenderPipeline::RenderPipeline(const std::shared_ptr<Windowing::Window>& window)
            : _window(window)
        {
            Windowing::Windowing::Subscribe(this);
        }

        RenderPipeline::~RenderPipeline()
        {
            Windowing::Windowing::UnSubscribe(this);
        }

        void RenderPipeline::Init()
        {
            const auto& render = Render::Instance();

            Texture2D::Description textureDescription;
            textureDescription.height = _window->GetHeight();
            textureDescription.width = _window->GetWidth();
            textureDescription.pixelFormat = PixelFormat::RGBA32F;

            auto const& hdrTexture = render->CreateTexture2D();
            hdrTexture->Init(textureDescription, nullptr);

            textureDescription.pixelFormat = PixelFormat::D16;
            auto const& depthTexture = render->CreateTexture2D();
            depthTexture->Init(textureDescription, nullptr);

            RenderTarget::RenderTargetDescription colorTarget;
            colorTarget.texture = hdrTexture;

            RenderTarget::RenderTargetDescription depthTarget;
            depthTarget.texture = depthTexture;
            depthTarget.isDepthTarget = true;

            _hdrRenderTargetContext = render->CreateRenderTargetContext();
            _hdrRenderTargetContext->SetColorTarget(RenderTargetIndex::INDEX_0, colorTarget);
            _hdrRenderTargetContext->SetDepthStencilTarget(depthTarget);

            _hdrRenderTargetContext->Bind();
            render->ClearColor(Vector4(0, 0, 0, 0));

            initPass<RenderPassOpaque>(*render, _hdrRenderTargetContext);
            initPass<RenderPassPostProcess>(*render, hdrTexture);
        }

        void RenderPipeline::Collect(const std::shared_ptr<SceneGraph>& sceneGraph)
        {
            getPass<RenderPassOpaque>()->Collect(sceneGraph);
            getPass<RenderPassPostProcess>()->Collect(sceneGraph);
        }

        void RenderPipeline::Draw()
        {
            getPass<RenderPassOpaque>()->Draw();
            getPass<RenderPassPostProcess>()->Draw();
        }

        void RenderPipeline::OnWindowResize(const Windowing::Window& window_)
        {
            int width = window_.GetWidth();
            int height = window_.GetHeight();
            _hdrRenderTargetContext->Resize(width, height);
        }
    }
}