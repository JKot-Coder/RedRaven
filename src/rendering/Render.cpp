#include "rendering/Render.hpp"
#include "resource_manager/ResourceManager.hpp"

namespace Rendering{

    void Render::Init() {
        initPass<RenderPassOpaque>(this);

        auto *resourceManager = ResourceManager::Instance().get();
        pbrShader = resourceManager->LoadShader("resources/test.shader");
    }

    void Render::Collect(const std::shared_ptr<SceneGraph>& sceneGraph) {
        getPass<RenderPassOpaque>()->Collect(sceneGraph);
    }

    void Render::Draw() {
        getPass<RenderPassOpaque>()->Draw();
    }

}