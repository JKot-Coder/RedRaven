#include "common/VecMath.h"
#include "common/Time.hpp"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Render.hpp"
#include "rendering/Camera.hpp"
#include "rendering/RenderContext.hpp"
#include "rendering/Primitives.hpp"

#include "Scene_2.hpp"

using namespace Common;

namespace Scenes {

    void Scene_2::Init() {
        const auto &resourceManager = ResourceManager::Instance();
        const auto scene = resourceManager->LoadScene("../assets/sponza.obj");

        lookAngle = vec2(0, 0);

        Rendering::Camera::Description cameraDescription;
        cameraDescription.zNear = 80;
        cameraDescription.zFar = 2500;
        cameraDescription.fow = 90;
        cameraDescription.orthoSize = 13;
        cameraDescription.isOrtho = false;

        camera = std::make_shared<Rendering::Camera>(cameraDescription);
        camera->LookAt(vec3(-3,-20,0), vec3(0,-20,0));

        renderElements = scene;
    }

    void Scenes::Scene_2::Collect(Rendering::RenderContext& renderContext) {
        renderContext.GetRenderQuery() = renderElements;
    }

    void Scene_2::Update() {
        auto dt = Time::Instance()->GetDeltaTime();
        auto const &inputting = Inputting::Instance().get();

        float speed = 200.0f;
        if (inputting->IsDown(Inputting::ikShift)){
            speed *= 2.0;
        }

        auto camTransform = camera->GetTransform();

        if (inputting->IsDown(Inputting::ikW)) {
            camTransform.Position += camTransform.Rotation * vec3(0, 0, -1) * speed * dt;
        } else if (inputting->IsDown(Inputting::ikS)) {
            camTransform.Position += camTransform.Rotation * vec3(0, 0, 1) * speed * dt;
        }

        if (inputting->IsDown(Inputting::ikD)) {
            camTransform.Position += camTransform.Rotation * vec3(1, 0, 0) * speed * dt;
        } else if (inputting->IsDown(Inputting::ikA)) {
            camTransform.Position += camTransform.Rotation * vec3(-1, 0, 0) * speed * dt;
        }

        lookAngle = lookAngle - inputting->Mouse.relative * 0.001f;
        lookAngle.y = clamp(lookAngle.y, -PI/2 + 0.1f, +PI/2 - 0.1f);

        vec3 dir = vec3(lookAngle.y, lookAngle.x) * 2.0;
        camTransform.Rotation = quat(dir, vec3(0, 1, 0));

        camera->SetTransform(camTransform);
    }

    std::shared_ptr<Rendering::Camera> Scene_2::GetMainCamera() {
        return camera;
    }

    void Scene_2::Terminate() {
        renderElements.clear();
    }
}
