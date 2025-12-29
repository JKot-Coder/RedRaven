#include "ShaderImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    ShaderImpl::~ShaderImpl() { }

    void ShaderImpl::Init(const wgpu::Device& device, const Shader& resource)
    {
        wgpu::ShaderModuleWGSLDescriptor wgslDesc;
        wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgslDesc.chain.next = nullptr;
        wgslDesc.code = wgpu::StringView(std::string_view(reinterpret_cast<const char*>(resource.GetDesc().data), resource.GetDesc().size));

        wgpu::ShaderModuleDescriptor desc;
        desc.setDefault();
        desc.label = wgpu::StringView(resource.GetName().c_str());
        desc.nextInChain = &wgslDesc.chain;

        shaderModule = device.createShaderModule(desc);
    }
}

