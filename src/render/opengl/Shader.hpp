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
        virtual ~Shader() override;

        virtual bool LinkSource(Common::Stream* stream) override;
    private:
        GLuint ID;

        bool checkLink();
    };

}
}