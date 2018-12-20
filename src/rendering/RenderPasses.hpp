#pragma once

#include <tuple>
#include <vector>
#include <memory>

#include "rendering/RenderContext.hpp"

namespace Rendering {

    class Render;
    class SceneGraph;

    enum class RenderPassType {
        OPAQUE,
        POST_PROCESS
    };

    class RenderPass {
    public:
        virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) = 0;
        virtual void Draw() = 0;
    };

    class RenderPassOpaque final: public RenderPass{
    public:
        RenderPassOpaque(Rendering::Render* render);

        virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) override;
        virtual void Draw() override;

    private:
        Render* render;
        std::unique_ptr<RenderContext> renderContext;
        std::shared_ptr<Camera> camera;
    };

}


