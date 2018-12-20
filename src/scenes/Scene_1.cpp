#include "common/VecMath.h"

#include "rendering/RenderContext.hpp"
#include "rendering/Primitives.hpp"

#include "Scene_1.hpp"

using namespace Common;

namespace Scenes {

    void Scenes::Scene_1::Collect(Rendering::RenderContext& renderContext) {
        renderContext.GetRenderQuery() = renderElements;
    }

    void Scene_1::Init() {
        sphereMesh = Rendering::Primitives::GetSphereMesh(23);

        mat4 modelMat;
        modelMat.identity();

        Rendering::RenderElement element;
        element.mesh = sphereMesh;
        element.modelMatrix = modelMat;

        renderElements.push_back(element);
    }

    void Scene_1::Terminate() {
        sphereMesh = nullptr;

        renderElements.clear();
    }

}
