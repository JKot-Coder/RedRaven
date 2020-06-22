#pragma once

#include "rendering/Texture.hpp"
#include "rendering/opengl/Render.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        namespace OpenGL
        {

            class Texture2D final : public Rendering::Texture2D
            {
            public:
                struct OpenGlPixelFormatDescription
                {
                    GLuint internalFormat, format;
                    GLenum type;
                };

                Texture2D();
                virtual ~Texture2D();

                virtual void Init(const Description& description, void* data) override;
                virtual void Bind(int sampler) override;

                inline virtual int GetWidth() const override { return _width; }
                inline virtual int GetHeight() const override { return _height; }

                virtual void Resize(int width, int height) override;

                inline GLuint GetNativeId() const { return _id; }

            private:
                GLuint _id;
                OpenGlPixelFormatDescription _pixelFormatDescription;
                int _width, _height;

                OpenGlPixelFormatDescription GetOpenGlPixelFormatDescription(PixelFormat pixelFormat) const;
            };

        }
    }
}