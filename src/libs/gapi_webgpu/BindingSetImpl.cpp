#include "BindingSetImpl.hpp"

#include "gapi/BindingLayout.hpp"

namespace RR::GAPI::WebGPU
{
    BindingSetImpl::~BindingSetImpl() { }

    void BindingSetImpl::Init(const wgpu::Device& device, GAPI::BindingSet& resource)
    {
        UNUSED(device);
        UNUSED(resource);


        // TODO: Implement BindingSet initialization
        // - Create BindGroupLayout from BindingSetLayout
        // - Create BindGroup with actual resources (buffers, textures, samplers)
        // - Map BindingType to WebGPU binding types
    }
}

