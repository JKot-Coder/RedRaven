#pragma once

#include "gapi/Shader.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class ShaderImpl final : public RR::GAPI::IShader
    {
    public:
        ShaderImpl() = default;
        ~ShaderImpl() override;

        void Init(const wgpu::Device& device, const Shader& resource);

        wgpu::ShaderModule GetShaderModule() const { return shaderModule; }

    private:
        wgpu::ShaderModule shaderModule;
    };
}

