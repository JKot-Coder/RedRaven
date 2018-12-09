#include "dependencies/glad/glad.h"

#include "common/Utils.hpp"
#include "common/Stream.hpp"

#include "rendering/opengl/Render.hpp"

#include "Shader.hpp"

namespace Rendering {
namespace OpenGL {

    Shader::Shader() : ID(glCreateProgram()) {

    }

    Shader::~Shader() {
        glDeleteProgram(ID);
    }

    bool Shader::LinkSource(Common::Stream *stream) {
        const char GLSL_VERT[] = "#define VERTEX\n";
        const char GLSL_FRAG[] = "#define FRAGMENT\n";

        auto shaderSize = stream->GetSize();
        char *text = new char[shaderSize + 1];
        text[shaderSize] = 0;

        stream->Read(text, shaderSize);

        const int type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
        const char *code[2][4] = {
                { "#version 330 core\n", GLSL_VERT, "#line 0\n", text },
                { "#version 330 core\n", GLSL_FRAG, "#line 0\n", text }
        };

        GLchar info[1024];

        for (int i = 0; i < 2; i++) {
            GLuint obj = glCreateShader(type[i]);
            glShaderSource(obj, 4, code[i], NULL);
            glCompileShader(obj);

            glGetShaderInfoLog(obj, sizeof(info), NULL, info);
            if (info[0])
                LOG("! shader: %s\n", info);

            glAttachShader(ID, obj);
            glDeleteShader(obj);
        }

        delete[] text;

//        for (int at = 0; at < aMAX; at++)
//            glBindAttribLocation(ID, at, AttribName[at]);

        glLinkProgram(ID);

        glGetProgramInfoLog(ID, sizeof(info), NULL, info);
        if (info[0]) LOG("! program: %s\n", info);

        if (!checkLink())
            return false;

        Bind();

        for (int ut = 0; ut < UNIFORM_TYPE_MAX; ut++)
            uniformID[ut] = glGetUniformLocation(ID, (GLchar*)UniformsNames[ut]);

        return true;
    }

    bool Shader::checkLink() const {
        GLint success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        return success != 0;
    }

    void Shader::Bind() const {
        glUseProgram(ID);
    }

    void Shader::SetParam(Shader::UniformType uType, const Common::vec4 &value, int count) const {
        if (uniformID[uType] != -1)
            glUniform4fv(uniformID[uType], count, (GLfloat*)&value);
    }

    void Shader::SetParam(Shader::UniformType uType, const Common::mat4 &value, int count) const {
        if (uniformID[uType] != -1)
            glUniformMatrix4fv(uniformID[uType], count, false, (GLfloat*)&value);
    }

    void Shader::SetParam(Shader::UniformType uType, const Common::Basis &value, int count) const {
        if (uniformID[uType] != -1)
            glUniform4fv(uniformID[uType], count * 2, (GLfloat*)&value);
    }

}
}