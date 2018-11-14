#include "Shader.hpp"

#include "render/opengl/Rendering.hpp"

namespace Render {
namespace OpenGL {

    Shader::Shader() : ID(glCreateProgram()) {

    }

    Shader::~Shader() {
        glDeleteProgram(ID);
    }

    void Shader::LinkSource(const Common::Stream& stream){
        (void) stream;
    }
}
}