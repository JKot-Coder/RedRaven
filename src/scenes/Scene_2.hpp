#pragma once

#include <memory>
#include <vector>

#include "common/VecMath.h"
#include "rendering/SceneGraph.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        class RenderContext;
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

            virtual void Collect(Rendering::RenderContext& renderContext) override;
            virtual std::shared_ptr<Rendering::Camera> GetMainCamera() override;

        private:
            Common::vec2 lookAngle;
            std::shared_ptr<Rendering::Camera> camera;
            std::vector<Rendering::RenderElement> renderElements;
        };
    }
}