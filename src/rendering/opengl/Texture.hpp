#pragma once

#include "rendering/opengl/Render.hpp"
#include "rendering/Texture.hpp"

namespace Rendering {
namespace OpenGL {

    class Texture2D final : public Rendering::Texture2D {
    public:

        struct OpenGlPixelFormatDescription {
            GLuint internalFormat, format;
            GLenum type;
        };

        Texture2D();
        virtual ~Texture2D();

        virtual void Init(const Description& description, void* data) override;
        virtual void Bind(int sampler) override;

        inline virtual int GetWidth() const override { return width; }
        inline virtual int GetHeight() const override { return height; }

        inline GLuint GetNativeId() const { return id; }
    private:
        GLuint id;
        int width, height;

        OpenGlPixelFormatDescription GetOpenGlPixelFormatDescription(PixelFormat pixelFormat) const;
    };

}
}