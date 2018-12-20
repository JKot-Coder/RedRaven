#pragma once

namespace Rendering {

    class CommonTexture {
    public:
        virtual void Bind(int sampler) = 0;
    };

    class Texture2D : virtual CommonTexture {
    public:

        struct Description {
            int width;
            int height;
            PixelFormat pixelFormat;
        };

        virtual void Init(const Description& description, void* data) = 0;
    };

}