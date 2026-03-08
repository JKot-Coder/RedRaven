#pragma once

#include "gapi/BindingGroup.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BindingGroupImpl final : public GAPI::IBindingGroup
    {
    public:
        BindingGroupImpl() = default;
        ~BindingGroupImpl();

        void Init(const wgpu::Device& device, const BindingGroupDesc& desc, wgpu::BindGroupLayout layout);

        wgpu::BindGroup GetBindGroup() const { return bindGroup; }

    private:
        wgpu::BindGroup bindGroup;
    };
}

