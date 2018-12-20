#pragma once

#include "rendering/opengl/Render.hpp"
#include "rendering/Texture.hpp"

namespace Rendering {
namespace OpenGL {

    class Texture2D final : virtual Rendering::Texture2D {
    public:

        struct OpenGlPixelFormatDescription {
            GLuint internalFormat, format;
            GLenum type;
        };

        Texture2D();
        virtual ~Texture2D();

        virtual void Init(const Description& description, void* data) override;

        virtual void Bind(int sampler) override;
    private:
        OpenGlPixelFormatDescription GetOpenGlPixelFormatDescription(PixelFormat pixelFormat) const;

        GLuint id;
    };

}
}