#pragma once

#include "gapi/BindingLayout.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BindingSetImpl final : public GAPI::IBindingSet
    {
    public:
        BindingSetImpl() = default;
        ~BindingSetImpl();

        void Init(const wgpu::Device& device, GAPI::BindingSet& resource);

        wgpu::BindGroup GetBindGroup() const { return bindGroup; }

    private:
        wgpu::BindGroup bindGroup;
    };
}

