#pragma once

#include <string>

#include "math/Math.hpp"

#include "rendering/Texture.hpp"

namespace OpenDemo
{
    namespace Common
    {
        class Stream;
    }

    namespace Rendering
    {
        namespace Uniform
        {
            enum Type
            {
                VIEW_PROJECTION_MATRIX,
                MODEL_MATRIX,
                CAMERA_POSITION,
                MATERIAL,
                LIGHT_DIR,
                UNIFORM_MAX
            };
        }

        class Shader
        {
        public:
            static const char* const UniformsNames[Uniform::UNIFORM_MAX];
            static const char* const SamplerNames[Sampler::SAMPLER_MAX];

            virtual ~Shader() {};

            virtual bool LinkSource(const std::shared_ptr<Stream>& stream) = 0;
            virtual void Bind() const = 0;

            virtual void SetParam(Uniform::Type uType, const Vector4& value, int count = 1) const = 0;
            virtual void SetParam(Uniform::Type uType, const Matrix4& value, int count = 1) const = 0;
            // virtual void SetParam(Uniform::Type uType, const Common::Basis& value, int count = 1) const = 0;
        };
    }
}