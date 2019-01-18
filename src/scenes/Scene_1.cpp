#include "common/VecMath.h"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Render.hpp"
#include "rendering/Camera.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/Primitives.hpp"

#include "Scene_1.hpp"

using namespace Common;

namespace Scenes {

    void Scene_1::Init() {
        Rendering::Camera::Description cameraDescription;
        cameraDescription.zNear = 1;
        cameraDescription.zFar = 1500;
        cameraDescription.fow = 90;
        cameraDescription.orthoSize = 13;
        cameraDescription.isOrtho = false;

        camera = std::make_shared<Rendering::Camera>(cameraDescription);
        camera->LookAt(vec3(-3,-20,0), vec3(0,-20,0));

        sphereMesh = Rendering::Primitives::GetSphereMesh(23);

        for (int i = 0; i < 8; i++) {
            mat4 modelMat;
            modelMat.identity();
            modelMat.translate(vec3(-8.0f + i * 2.2f, 0.0f, 0.0f));

            Rendering::Material material;

            Rendering::RenderElement element;
            element.mesh = sphereMesh;
            element.modelMatrix = modelMat;
            element.material = material;

            renderElements.push_back(element);
        }
    }

    void Scenes::Scene_1::Collect(Rendering::RenderContext& renderContext) {
        renderContext.GetRenderQuery() = renderElements;
    }

    std::shared_ptr<Rendering::Camera> Scene_1::GetMainCamera() {
        return camera;
    }

    void Scene_1::Terminate() {
        sphereMesh = nullptr;

        renderElements.clear();
    }

}
