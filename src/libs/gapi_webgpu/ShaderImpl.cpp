#include "ShaderImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    ShaderImpl::~ShaderImpl() { }

    void ShaderImpl::Init(const wgpu::Device& device, const Shader& resource)
    {
        UNUSED(device, resource);
        NOT_IMPLEMENTED();
    }
}

