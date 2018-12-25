#include "resource_manager/ResourceManager.hpp"

#include "rendering/Texture.hpp"
#include "rendering/RenderTarget.hpp"
#include "rendering/RenderTargetContext.hpp"

#include "rendering/Render.hpp"

namespace Rendering{

    void Render::Init() {
        Texture2D::Description textureDescription;
        textureDescription.height = 512;
        textureDescription.width  = 512;
        textureDescription.pixelFormat = PixelFormat::RGBA8;

        auto const &hdrTexture = CreateTexture2D();
        hdrTexture->Init(textureDescription, nullptr);

        RenderTarget::RenderTargetDescription rtd;
        rtd.texture = hdrTexture;

        auto const &hdrRenderTargetContext = CreateRenderTargetContext();
        hdrRenderTargetContext->SetColorTarget(RenderTargetIndex::INDEX_0, rtd);

        initPass<RenderPassOpaque>(this, hdrRenderTargetContext);
        initPass<RenderPassPostProcess>(this, hdrTexture);
    }

    void Render::Collect(const std::shared_ptr<SceneGraph> &sceneGraph) {
        getPass<RenderPassOpaque>()->Collect(sceneGraph);
        getPass<RenderPassPostProcess>()->Collect(sceneGraph);
    }

    void Render::Draw() {
        getPass<RenderPassOpaque>()->Draw();
        getPass<RenderPassPostProcess>()->Draw();
    }

}