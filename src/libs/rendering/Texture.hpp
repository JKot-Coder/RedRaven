#pragma once

namespace OpenDemo
{
    namespace Rendering
    {
        enum PixelFormat : int;

        namespace Sampler
        {
            enum Type
            {
                ALBEDO,
                NORMAL,
                ROUGHNESS,
                METALLIC,
                SAMPLER_MAX
            };
        };

        class CommonTexture
        {
        public:
            virtual int GetWidth() const = 0;
            virtual int GetHeight() const = 0;

            virtual void Resize(int width, int height) = 0;

            virtual void Bind(int sampler) = 0;
        };

        class Texture2D : public CommonTexture
        {
        public:
            struct Description
            {
                int width;
                int height;
                PixelFormat pixelFormat;
            };

            virtual void Init(const Description& description, void* data) = 0;
        };
    }
}