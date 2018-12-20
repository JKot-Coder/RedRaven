#pragma once

#include "rendering/Render.hpp"
#include "rendering/Texture.hpp"

namespace Rendering {

    enum class RenderTargetIndex {
        INDEX_0,
        INDEX_1,
        INDEX_MAX
    };

    class CommonRenderTarget {
    };

    class RenderTargetTexture : virtual CommonTexture, virtual CommonRenderTarget {
    public:

        struct RenderTargetDescription {
            int width;
            int height;
            PixelFormat pixelFormat;
        };

        virtual ~RenderTargetTexture() {};

        virtual void Init(const RenderTargetDescription& description) = 0;
    };

}
