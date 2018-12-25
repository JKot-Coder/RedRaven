#include "rendering/Shader.hpp"

namespace Rendering {

    const char* const Shader::UniformsNames[Shader::UNIFORM_TYPE_MAX] = { "ViewProjection", "Model", "CameraPosition", "Material" };
    const char* const Shader::SamplerNames [Shader::SAMPLER_TYPE_MAX] = { "AlbedoTex" };

}