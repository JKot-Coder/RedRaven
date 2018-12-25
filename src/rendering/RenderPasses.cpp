#include "common/VecMath.h"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Render.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/SceneGraph.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/RenderTarget.hpp"
#include "rendering/RenderTargetContext.hpp"
#include "rendering/RenderPasses.hpp"

using namespace Common;

namespace Rendering {

    RenderPassOpaque::RenderPassOpaque(Rendering::Render* render, const std::shared_ptr<RenderTargetContext> &hdrRenderTargetContext):
        render(render),
        hdrRenderTargetContext(hdrRenderTargetContext),
        renderContext(new RenderContext())
    {
        auto *resourceManager = ResourceManager::Instance().get();
        pbrShader = resourceManager->LoadShader("resources/pbr.shader");

        renderContext->SetShader(pbrShader);

        Camera::Description cameraDescription;
        cameraDescription.zNear = 1;
        cameraDescription.zFar = 100;
        cameraDescription.fow = 90;
        cameraDescription.isOrtho = false;

        camera = std::make_shared<Camera>(cameraDescription);
        camera->LookAt(vec3(0,0,-8), vec3(0,0,0));

        renderContext->SetCamera(camera);
        renderContext->SetRenderTarget(hdrRenderTargetContext);

        renderContext->SetDepthWrite(true);
        renderContext->SetDepthTestFunction(ALWAYS);
    }

    void RenderPassOpaque::Collect(const std::shared_ptr<SceneGraph>& sceneGraph) {
        sceneGraph->Collect(*renderContext);
    }

    void RenderPassOpaque::Draw() {
        camera->SetAspect(1024, 768);

        render->Begin(renderContext);

        render->Clear(vec4(0.25, 0.25, 0.25, 0), 1.0);

        const auto& renderQuery = renderContext->GetRenderQuery();
        for (const auto& item : renderQuery){
           render->DrawElement(item);
        }

        render->End();
    }

    RenderPassPostProcess::RenderPassPostProcess(Rendering::Render *render, const std::shared_ptr<Texture2D> &hdrTexture) :
            render(render),
            hdrTexture(hdrTexture),
            renderContext(new RenderContext())
    {
        (void) render;
        auto *resourceManager = ResourceManager::Instance().get();
        postProcessShader = resourceManager->LoadShader("resources/postProcess.shader");

        renderContext->SetShader(postProcessShader);
        renderContext->SetDepthWrite(false);
        renderContext->SetDepthTestFunction(DepthTestFunction::ALWAYS);

        fullScreenQuad = Primitives::GetFullScreenQuad();
    }

    void RenderPassPostProcess::Collect(const std::shared_ptr<SceneGraph> &sceneGraph) {
        (void) sceneGraph;

    }

    void RenderPassPostProcess::Draw() {
        render->Begin(renderContext);
        hdrTexture->Bind(0);

        fullScreenQuad->Draw();

        render->End();
    }
}
