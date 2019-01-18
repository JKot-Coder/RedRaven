#pragma once

#include <string>

#include "common/VecMath.h"

#include "rendering/Texture.hpp"

namespace Common{
    class Stream;
}

namespace Rendering
{
    namespace Uniform {
        enum Type {
            VIEW_PROJECTION_MATRIX,
            MODEL_MATRIX,
            CAMERA_POSITION,
            MATERIAL,
            LIGHT_DIR,
            UNIFORM_MAX
        };
    }

    class Shader {
    public:

        static const char* const UniformsNames[Uniform::UNIFORM_MAX];
        static const char* const SamplerNames[Sampler::SAMPLER_MAX];

        virtual ~Shader() {};

        virtual bool LinkSource(const std::shared_ptr<Common::Stream> &stream) = 0;
        virtual void Bind() const = 0;

        virtual void SetParam(Uniform::Type uType, const Common::vec4 &value, int count = 1) const = 0;
        virtual void SetParam(Uniform::Type uType, const Common::mat4 &value, int count = 1) const = 0;
        virtual void SetParam(Uniform::Type uType, const Common::Basis &value, int count = 1) const = 0;
    };

}