#include "rendering/Shader.hpp"

namespace Rendering {

    const char* const Shader::UniformsNames[Uniform::UNIFORM_MAX] = { "ViewProjection", "Model", "CameraPosition", "Material", "LightDirection" };
    const char* const Shader::SamplerNames [Sampler::SAMPLER_MAX] = { "AlbedoMap", "NormalMap", "RoughnessMap", "MetallicMap" };

}