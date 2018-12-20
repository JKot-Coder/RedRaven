#include "rendering/Render.hpp"

#include "common/VecMath.h"

#include "rendering/RenderPasses.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/SceneGraph.hpp"
#include "rendering/Camera.hpp"

using namespace Common;

namespace Rendering {

    RenderPassOpaque::RenderPassOpaque(Rendering::Render* render) :
        render(render),
        renderContext(new RenderContext())
    {
        Camera::Description cameraDescription;
        cameraDescription.zNear = 1;
        cameraDescription.zFar = 100;
        cameraDescription.fow = 90;
        cameraDescription.isOrtho = false;

        camera = std::make_shared<Camera>(cameraDescription);
        camera->LookAt(vec3(0,0,-8), vec3(0,0,0));

        renderContext->SetCamera(camera);
    }

    void RenderPassOpaque::Collect(const std::shared_ptr<SceneGraph>& sceneGraph) {
        sceneGraph->Collect(*renderContext);
    }

    void RenderPassOpaque::Draw() {
        camera->SetAspect(1024, 768);
        render->Clear(vec4(0.25, 0.25, 0.25, 0), 1.0);

        const auto& renderQuery = renderContext->GetRenderQuery();
        for (const auto& item : renderQuery){
           render->DrawElement(*renderContext, item);
        }
    }

}
