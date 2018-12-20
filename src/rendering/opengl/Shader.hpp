#pragma once

#include "common/VecMath.h"

#include "rendering/Shader.hpp"

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

        virtual void SetParam(UniformType uType, const Common::vec4 &value, int count = 1) const override;
        virtual void SetParam(UniformType uType, const Common::mat4 &value, int count = 1) const override;
        virtual void SetParam(UniformType uType, const Common::Basis &value, int count = 1) const override;
    private:
        GLuint id;
        GLint uniformID[UNIFORM_TYPE_MAX];
        bool checkLink() const;
    };

}
}