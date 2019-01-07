#include "common/VecMath.h"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/RenderContext.hpp"
#include "rendering/Primitives.hpp"

#include "Scene_2.hpp"

using namespace Common;

namespace Scenes {

    void Scenes::Scene_2::Collect(Rendering::RenderContext& renderContext) {
        renderContext.GetRenderQuery() = renderElements;
    }

    void Scene_2::Init() {
        const auto &resourceManager = ResourceManager::Instance();
        const auto meshes = resourceManager->LoadScene("../../assets/sponza.obj");

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

    void Scene_2::Terminate() {
        renderElements.clear();
    }

}
