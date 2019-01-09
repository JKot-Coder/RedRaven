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

    RenderPassOpaque::RenderPassOpaque(Rendering::Render& render, const std::shared_ptr<RenderTargetContext> &hdrRenderTargetContext):
        render(&render),
        hdrRenderTargetContext(hdrRenderTargetContext),
        renderContext(new RenderContext())
    {
        auto *resourceManager = ResourceManager::Instance().get();
        pbrShader = resourceManager->LoadShader("resources/pbr.shader");

        renderContext->SetShader(pbrShader);
        renderContext->SetRenderTarget(hdrRenderTargetContext);

        renderContext->SetDepthWrite(true);
        renderContext->SetDepthTestFunction(LEQUAL);

        BlendingDescription blendingDescription(BlendingMode::ADDITIVE);

        renderContext->SetBlending(false);
        renderContext->SetBlendingDescription(blendingDescription);
    }

    void RenderPassOpaque::Collect(const std::shared_ptr<SceneGraph>& sceneGraph) {
        auto const &camera = sceneGraph->GetMainCamera();
        camera->SetAspect(1024, 768);

        sceneGraph->Collect(*renderContext);

        renderContext->SetCamera(camera);
    }

    vec3 RandomRay() {
        float theta, cosphi, sinphi;

        theta = 2.0f * PI * std::rand() /static_cast <float>(RAND_MAX);
        cosphi = 1.0f - 2.0f * std::rand() /static_cast <float>(RAND_MAX);
        sinphi = sqrt(1.0f - min(1.0f, cosphi * cosphi));

        vec3 result;

        result.x = sinphi * cos(theta);
        result.y = sinphi * sin(theta);
        result.z = cosphi;

        return result;
    }

    void RenderPassOpaque::Draw() {
        //vec3 lightDirection = vec3( std::rand() /static_cast <float>(RAND_MAX) * PI * 2.0,  std::rand() /static_cast <float>(RAND_MAX) *PI* 2.0);
        vec3 lightDirection = RandomRay();

        renderContext->SetLightDirection(lightDirection);

        render->Begin(renderContext);

        render->ClearDepthStencil(true);
        render->Clear(vec4(0.0, 0.0, 0.0, 0), 1.0);
        //render->Clear(vec4(0.25, 0.25, 0.25, 0), 1.0);

        const auto& renderQuery = renderContext->GetRenderQuery();
        for (const auto& item : renderQuery){
           render->DrawElement(item);
        }

        render->End();
    }

    RenderPassPostProcess::RenderPassPostProcess(Rendering::Render& render, const std::shared_ptr<Texture2D> &hdrTexture) :
            render(&render),
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
