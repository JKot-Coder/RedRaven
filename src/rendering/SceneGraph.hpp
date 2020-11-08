#pragma once


namespace OpenDemo
{
    namespace Rendering
    {

        class RenderCommandContext;
        class Camera;

        class SceneGraph
        {
        public:
            virtual void Init() = 0;
            virtual void Terminate() = 0;
            virtual void Update() = 0;

            virtual void Collect(RenderCommandContext& RenderCommandContext) = 0;
            virtual std::shared_ptr<Camera> GetMainCamera() = 0;
        };

    }
}