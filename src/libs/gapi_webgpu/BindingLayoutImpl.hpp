#pragma once

#include "gapi/BindingLayout.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BindingLayoutImpl final : public GAPI::IBindingLayout
    {
    public:
        BindingLayoutImpl() = default;
        ~BindingLayoutImpl();

        void Init(const wgpu::Device& device, GAPI::BindingLayout& resource);

    private:
        eastl::fixed_vector<wgpu::BindGroupLayout, GAPI::MAX_BINDING_GROUPS, false> bindGroupLayouts;
    };
}

