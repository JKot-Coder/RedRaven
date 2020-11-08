#pragma once

#include "common/Math.hpp"
#include "rendering/SceneGraph.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        class RenderCommandContext;
        class Mesh;
        class Camera;
        struct RenderElement;
    }

    namespace Scenes
    {
        class Scene_2 final : public Rendering::SceneGraph
        {
        public:
            virtual void Init() override;
            virtual void Terminate() override;
            virtual void Update() override;

            virtual void Collect(Rendering::RenderCommandContext& RenderCommandContext) override;
            virtual std::shared_ptr<Rendering::Camera> GetMainCamera() override;

        private:
            Common::Vector2 _lookAngle;
            std::shared_ptr<Rendering::Camera> _camera;
            std::vector<Rendering::RenderElement> _renderElements;
        };
    }
}