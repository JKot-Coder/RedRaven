#pragma once

#include <array>

#include "common/VecMath.h"

#include "rendering/Shader.hpp"

#include "rendering/opengl/Render.hpp"

namespace OpenDemo
{
    namespace Common
    {
        class Stream;
    }

    namespace Rendering
    {
        namespace OpenGL
        {

            class Shader final : public Rendering::Shader
            {
            public:
                Shader();
                virtual ~Shader() override;

                virtual bool LinkSource(const std::shared_ptr<Common::Stream>& stream) override;
                virtual void Bind() const override;

                virtual void SetParam(Uniform::Type uType, const Common::vec4& value, int count = 1) const override;
                virtual void SetParam(Uniform::Type uType, const Common::mat4& value, int count = 1) const override;
                virtual void SetParam(Uniform::Type uType, const Common::Basis& value, int count = 1) const override;

            private:
                GLuint id;
                std::array<GLint, Uniform::UNIFORM_MAX> uniformID = {};
                bool checkLink() const;
            };

        }
    }
}