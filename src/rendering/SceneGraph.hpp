#pragma once

#include <memory>

namespace Rendering {

    class RenderContext;

    class SceneGraph {
    public:
        virtual void Init() = 0;
        virtual void Terminate() = 0;

        virtual void Collect(RenderContext& renderContext) = 0;
    };

}

