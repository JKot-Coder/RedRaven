#pragma once

#include "render/opengl/Rendering.hpp"
#include "render/Shader.hpp"

namespace Common {
    class Stream;
}

namespace Render {
namespace OpenGL {

    class Shader : public Render::Shader {
    public:
        Shader();
        ~Shader();

        void LinkSource(const Common::Stream& stream);
    private:
        GLuint ID;
    };

}
}