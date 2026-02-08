#include "BindingGroupImpl.hpp"

#include "gapi/BindingLayout.hpp"

namespace RR::GAPI::WebGPU
{
    BindingGroupImpl::~BindingGroupImpl() { }

    void BindingGroupImpl::Init(const wgpu::Device& device, GAPI::BindingGroup& resource, BindingGroupDesc& desc)
    {
        UNUSED(device);
        UNUSED(resource);
        UNUSED(desc);


        // TODO: Implement BindingSet initialization
        // - Create BindGroupLayout from BindingSetLayout
        // - Create BindGroup with actual resources (buffers, textures, samplers)
        // - Map BindingType to WebGPU binding types
    }
}

