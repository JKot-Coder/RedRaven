#include "RenderPasses.hpp"

#include "common/Math.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Camera.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/Render.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/RenderTarget.hpp"
#include "rendering/RenderTargetContext.hpp"
#include "rendering/SceneGraph.hpp"
#include "rendering/Shader.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace Rendering
    {
        RenderPassOpaque::RenderPassOpaque(Rendering::Render& render, const std::shared_ptr<RenderTargetContext>& hdrRenderTargetContext)
            : _render(&render)
            , _hdrRenderTargetContext(hdrRenderTargetContext)
            , _renderContext(new RenderContext())
        {
            auto* resourceManager = ResourceManager::Instance().get();
            _pbrShader = resourceManager->LoadShader("../../assets/shaders/pbr.shader");

            _renderContext->SetShader(_pbrShader);
            _renderContext->SetRenderTarget(hdrRenderTargetContext);

            _renderContext->SetDepthWrite(true);
            _renderContext->SetDepthTestFunction(LEQUAL);

            BlendingDescription blendingDescription(BlendingMode::ADDITIVE);

            _renderContext->SetBlending(false);
            _renderContext->SetBlendingDescription(blendingDescription);
        }

        void RenderPassOpaque::Collect(const std::shared_ptr<SceneGraph>& sceneGraph)
        {
            auto const& camera = sceneGraph->GetMainCamera();
            camera->SetAspect(1024, 768);

            sceneGraph->Collect(*_renderContext);

            _renderContext->SetCamera(camera);
        }

        Vector3 RandomRay()
        {
            float theta, cosphi, sinphi;

            theta = 2.0f * PI * std::rand() / static_cast<float>(RAND_MAX);
            cosphi = 1.0f - 2.0f * std::rand() / static_cast<float>(RAND_MAX);
            sinphi = (float)sqrt(1.0f - Min(1.0f, cosphi * cosphi));

            Vector3 result(
                sinphi * (float)cos(theta),
                sinphi * (float)sin(theta),
                cosphi
            );

            return result;
        }

        void RenderPassOpaque::Draw()
        {
            //Vector3 lightDirection = Vector3( std::rand() /static_cast <float>(RAND_MAX) * PI * 2.0,  std::rand() /static_cast <float>(RAND_MAX) *PI* 2.0);
            Vector3 lightDirection = RandomRay();

            _renderContext->SetLightDirection(lightDirection);

            _render->Begin(_renderContext);

            _render->ClearDepthStencil(true);
            _render->Clear(Vector4(0.0, 0.0, 0.0, 0), 1.0);
            //render->Clear(Vector4(0.25, 0.25, 0.25, 0), 1.0);

            const auto& renderQuery = _renderContext->GetRenderQuery();
            for (const auto& item : renderQuery)
            {
                _render->DrawElement(item);
            }

            _render->End();
        }

        RenderPassPostProcess::RenderPassPostProcess(Rendering::Render& render, const std::shared_ptr<Texture2D>& hdrTexture)
            : _render(&render)
            , _hdrTexture(hdrTexture)
            , _renderContext(new RenderContext())
        {
            (void)render;
            auto* resourceManager = ResourceManager::Instance().get();
            _postProcessShader = resourceManager->LoadShader("../../assets/shaders/postProcess.shader");

            _renderContext->SetShader(_postProcessShader);
            _renderContext->SetDepthWrite(false);
            _renderContext->SetDepthTestFunction(DepthTestFunction::ALWAYS);

            _fullScreenQuad = Primitives::GetFullScreenQuad();
        }

        void RenderPassPostProcess::Collect(const std::shared_ptr<SceneGraph>& sceneGraph)
        {
            (void)sceneGraph;
        }

        void RenderPassPostProcess::Draw()
        {
            _render->Begin(_renderContext);
            _hdrTexture->Bind(0);

            _fullScreenQuad->Draw();

            _render->End();
        }
    }
}