#include "common/VecMath.h"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Camera.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/Primitives.hpp"

#include "Scene_2.hpp"

using namespace Common;

namespace Scenes {

    void Scene_2::Init() {
        const auto &resourceManager = ResourceManager::Instance();
        const auto meshes = resourceManager->LoadScene("../../assets/sponza.obj");

        Rendering::Camera::Description cameraDescription;
        cameraDescription.zNear = 1;
        cameraDescription.zFar = 1500;
        cameraDescription.fow = 90;
        cameraDescription.orthoSize = 13;
        cameraDescription.isOrtho = false;

        camera = std::make_shared<Rendering::Camera>(cameraDescription);
        camera->LookAt(vec3(-3,-20,0), vec3(0,-20,0));

        mat4 modelMat;
        modelMat.identity();

        Rendering::Material material;
        material.albedo = vec3(1.0, 1.0, 1.0);
        material.roughness = 0.8;

        for (size_t i = 0; i < meshes.size(); ++i) {
            Rendering::RenderElement element;
            element.mesh = meshes[i];
            element.modelMatrix = modelMat;
            element.material = material;

            renderElements.push_back(element);
        }
    }

    void Scenes::Scene_2::Collect(Rendering::RenderContext& renderContext) {
        renderContext.GetRenderQuery() = renderElements;
    }

    void Scene_2::Update() {
        auto const &inputting = Inputting::Instance().get();

        auto camTransform = camera->GetTransform();

        if (inputting->IsDown(Inputting::ikW)) {
            camTransform.Position = camTransform.Position + vec3(0.001, 0.0, 0.0);
        }

        camera->SetTransform(camTransform);
    }

    std::shared_ptr<Rendering::Camera> Scene_2::GetMainCamera() {
        return camera;
    }

    void Scene_2::Terminate() {
        renderElements.clear();
    }
}
