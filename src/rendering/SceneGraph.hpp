#pragma once

#include <memory>

namespace Rendering {

    class RenderContext;
    class Camera;

    class SceneGraph {
    public:
        virtual void Init() = 0;
        virtual void Terminate() = 0;
        virtual void Update() = 0;

        virtual void Collect(RenderContext& renderContext) = 0;
        virtual std::shared_ptr<Camera> GetMainCamera() = 0;
    };

}

