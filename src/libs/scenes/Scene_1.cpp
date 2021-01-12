#include "Scene_1.hpp"

#include "common/Math.hpp"

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
        void Scene_1::Init()
        {
            Rendering::Camera::Description cameraDescription;
            cameraDescription.zNear = 1;
            cameraDescription.zFar = 1500;
            cameraDescription.fow = 90;
            cameraDescription.orthoSize = 13;
            cameraDescription.isOrtho = false;

            _camera = std::make_shared<Rendering::Camera>(cameraDescription);
            _camera->LookAt(Vector3(-3, -20, 0), Vector3(0, -20, 0));

            _sphereMesh = Rendering::Primitives::GetSphereMesh(23);

            for (int i = 0; i < 8; i++)
            {
                Matrix4 modelMat(Identity);
                modelMat.Translate(Vector3(-8.0f + i * 2.2f, 0.0f, 0.0f));

                Rendering::Material material;

                Rendering::RenderElement element;
                element.mesh = _sphereMesh;
                element.modelMatrix = modelMat;
                element.material = material;

                _renderElements.push_back(element);
            }
        }

        void Scenes::Scene_1::Collect(Rendering::RenderContext& renderContext)
        {
            renderContext.GetRenderQuery() = _renderElements;
        }

        std::shared_ptr<Rendering::Camera> Scene_1::GetMainCamera()
        {
            return _camera;
        }

        void Scene_1::Terminate()
        {
            _sphereMesh = nullptr;

            _renderElements.clear();
        }
    }
}