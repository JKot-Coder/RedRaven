#pragma once

#include "common/VecMath.h"

#include "rendering/Shader.hpp"

#include "rendering/opengl/Render.hpp"

namespace Common {
    class Stream;
}

namespace Rendering {
namespace OpenGL {

    class Shader final : public Rendering::Shader {
    public:
        Shader();
        virtual ~Shader() override;

        virtual bool LinkSource(Common::Stream *stream) override;
        virtual void Bind() const override;

        virtual void SetParam(Uniform::Type uType, const Common::vec4 &value, int count = 1) const override;
        virtual void SetParam(Uniform::Type uType, const Common::mat4 &value, int count = 1) const override;
        virtual void SetParam(Uniform::Type uType, const Common::Basis &value, int count = 1) const override;
    private:
        GLuint id;
        GLint uniformID[Uniform::UNIFORM_MAX];
        bool checkLink() const;
    };

}
}