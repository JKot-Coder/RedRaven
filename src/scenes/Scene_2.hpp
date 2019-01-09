#pragma once

#include <vector>
#include <memory>

#include "rendering/SceneGraph.hpp"

namespace Rendering {
    class RenderContext;
    class Mesh;
    class Camera;
    struct RenderElement;
}

namespace Scenes {

    class Scene_2 final : public Rendering::SceneGraph {
    public:
        virtual void Init() override;
        virtual void Terminate() override;
        virtual void Update() override;

        virtual void Collect(Rendering::RenderContext& renderContext) override;
        virtual std::shared_ptr<Rendering::Camera> GetMainCamera() override;
    private:
        std::shared_ptr<Rendering::Camera> camera;
        std::vector<Rendering::RenderElement> renderElements;
    };
}


