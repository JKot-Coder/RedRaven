#pragma once

#include <memory>
#include <vector>

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

        class Scene_1 final : public Rendering::SceneGraph
        {
        public:
            virtual void Init() override;
            virtual void Terminate() override;
            virtual inline void Update() override { }

            virtual void Collect(Rendering::RenderContext& renderContext) override;
            virtual std::shared_ptr<Rendering::Camera> GetMainCamera() override;

        private:
            std::shared_ptr<Rendering::Camera> camera;
            std::shared_ptr<Rendering::Mesh> sphereMesh;
            std::vector<Rendering::RenderElement> renderElements;
        };
    }
}