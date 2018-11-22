#pragma once

#include <cstdint>
#include "render/Shader.hpp"

namespace Common {
    class Stream;
}

namespace Render {
namespace OpenGL {

    typedef uint16_t GLuint;

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