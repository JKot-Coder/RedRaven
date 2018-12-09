#pragma once

#include <tuple>
#include <memory>

namespace Rendering {

    enum class RenderPassType {
        OPAQUE,
        POST_PROCESS
    };

    class RenderPass {
    public:
        virtual void Collect() = 0;
        virtual void Draw() = 0;
    };

    class RenderPassOpaque: public RenderPass{
    public:
        virtual void Collect() override;
        virtual void Draw() override;
    };

}


