#pragma once

#include "gapi/BindingLayout.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BindingGroupLayoutImpl final : public GAPI::IBindingGroupLayout
    {
    public:
        BindingGroupLayoutImpl() = default;
        ~BindingGroupLayoutImpl();

        void Init(const wgpu::Device& device, GAPI::BindingGroupLayout& resource);

    private:
        wgpu::BindGroupLayout bindGroupLayout;
    };
}

