#include "Scene_2.hpp"

#include "common/Time.hpp"
#include "common/Math.hpp"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Camera.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/Render.hpp"
#include "rendering/RenderContext.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace Scenes
    {

        void Scene_2::Init()
        {
            const auto& resourceManager = ResourceManager::Instance();
            const auto scene = resourceManager->LoadScene("../../assets/sponza.obj");

            _lookAngle = Vector2(0, 0);

            Rendering::Camera::Description cameraDescription;
            cameraDescription.zNear = 80;
            cameraDescription.zFar = 2500;
            cameraDescription.fow = 90;
            cameraDescription.orthoSize = 13;
            cameraDescription.isOrtho = false;

            _camera = std::make_shared<Rendering::Camera>(cameraDescription);
            _camera->LookAt(Vector3(-3, -20, 0), Vector3(0, -20, 0));

            _renderElements = scene;
        }

        void Scenes::Scene_2::Collect(Rendering::RenderContext& renderContext)
        {
            renderContext.GetRenderQuery() = _renderElements;
        }

        void Scene_2::Update()
        {
            auto dt = Time::Instance()->GetDeltaTime();
            auto const& inputting = Inputting::Instance().get();

            float speed = 200.0f;
            if (inputting->IsDown(Inputting::ikShift))
            {
                speed *= 2.0;
            }

            auto camTransform = _camera->GetTransform();

            if (inputting->IsDown(Inputting::ikW))
            {
                camTransform.Position += camTransform.Rotation * Vector3(0, 0, -1) * speed * dt;
            }
            else if (inputting->IsDown(Inputting::ikS))
            {
                camTransform.Position += camTransform.Rotation * Vector3(0, 0, 1) * speed * dt;
            }

            if (inputting->IsDown(Inputting::ikD))
            {
                camTransform.Position += camTransform.Rotation * Vector3(1, 0, 0) * speed * dt;
            }
            else if (inputting->IsDown(Inputting::ikA))
            {
                camTransform.Position += camTransform.Rotation * Vector3(-1, 0, 0) * speed * dt;
            }

            _lookAngle = _lookAngle - inputting->Mouse.relative.cast<float>() * 0.001f;
            _lookAngle.y() = Clamp(_lookAngle.y(), -PI / 2 + 0.1f, +PI / 2 - 0.1f);

               ASSERT(false);
            Vector3 dir;//           = Vector3(_lookAngle.y(), _lookAngle.x()) * 2.0;
           
         
            // camTransform.Rotation = Quaternion(dir, Vector3(0, 1, 0));

            _camera->SetTransform(camTransform);
        }

        std::shared_ptr<Rendering::Camera> Scene_2::GetMainCamera()
        {
            return _camera;
        }

        void Scene_2::Terminate()
        {
            _renderElements.clear();
        }
    }
}