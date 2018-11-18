#include <common/Utils.hpp>
#include "Shader.hpp"

#include "common/Stream.hpp"

#include "render/opengl/Rendering.hpp"

namespace Render {
namespace OpenGL {

    Shader::Shader() : ID(glCreateProgram()) {

    }

    Shader::~Shader() {
        glDeleteProgram(ID);
    }

    bool Shader::LinkSource(Common::Stream* stream) {
        const char GLSL_VERT[] = "#define VERTEX\n";
        const char GLSL_FRAG[] = "#define FRAGMENT\n";

        auto shaderSize = stream->GetSize();
        char *text = new char(shaderSize);

        stream->Read(text, shaderSize);

        const int type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
        const char *code[2][4] = {
                { GLSL_VERT , "#line 0\n", text },
                { GLSL_FRAG , "#line 0\n", text }
        };

        GLchar info[1024];

        for (int i = 0; i < 2; i++) {
            GLuint obj = glCreateShader(type[i]);
            glShaderSource(obj, 4, code[i], NULL);
            glCompileShader(obj);

            glGetShaderInfoLog(obj, sizeof(info), NULL, info);
            if (info[0]) LOG("! shader: %s\n", info);

            glAttachShader(ID, obj);
            glDeleteShader(obj);
        }

       // for (int at = 0; at < aMAX; at++)
        //    glBindAttribLocation(ID, at, AttribName[at]);

        glLinkProgram(ID);

        glGetProgramInfoLog(ID, sizeof(info), NULL, info);
        if (info[0]) LOG("! program: %s\n", info);

        return checkLink();
    }

    bool Shader::checkLink() {
        GLint success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        return success != 0;
    }

}
}