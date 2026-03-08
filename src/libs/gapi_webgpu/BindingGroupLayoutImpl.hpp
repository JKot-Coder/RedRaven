#pragma once

#include "gapi/BindingGroupLayout.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BindingGroupLayoutImpl final : public GAPI::IBindingGroupLayout
    {
    public:
        BindingGroupLayoutImpl() = default;
        ~BindingGroupLayoutImpl();

        void Init(const wgpu::Device& device, const GAPI::BindingGroupLayoutDesc& desc, GAPI::BindingGroupLayout& resource);

        wgpu::BindGroupLayout GetBindGroupLayout() const { return bindGroupLayout; }

    private:
        wgpu::BindGroupLayout bindGroupLayout;
    };
}

