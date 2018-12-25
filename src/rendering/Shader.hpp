#pragma once

#include <string>

#include "common/VecMath.h"

namespace Common{
    class Stream;
}

namespace Rendering
{

    class Shader {
    public:

        enum UniformType {
            VIEW_PROJECTION_MATRIX,
            MODEL_MATRIX,
            CAMERA_POSITION,
            MATERIAL,
            UNIFORM_TYPE_MAX
        };

        enum SamplerType {
            ALBEDO,
            SAMPLER_TYPE_MAX
        };

        static const char* const UniformsNames[UniformType::UNIFORM_TYPE_MAX];
        static const char* const SamplerNames[SamplerType::SAMPLER_TYPE_MAX];

        virtual ~Shader() {};

        virtual bool LinkSource(Common::Stream *stream) = 0;
        virtual void Bind() const = 0;

        virtual void SetParam(UniformType uType, const Common::vec4 &value, int count = 1) const = 0;
        virtual void SetParam(UniformType uType, const Common::mat4 &value, int count = 1) const = 0;
        virtual void SetParam(UniformType uType, const Common::Basis &value, int count = 1) const = 0;
    };

}