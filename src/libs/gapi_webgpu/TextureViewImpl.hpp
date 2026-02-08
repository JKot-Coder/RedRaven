#pragma once

#include "gapi/GpuResourceViews.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class TextureViewImpl final : public IGpuResourceView
    {
    public:
        TextureViewImpl() = default;
        ~TextureViewImpl();

        void Init(GAPI::GpuResourceView& resource);
        void DestroyResource();

        wgpu::TextureView GetTextureView() const { return view; }

        void RecreateView(GAPI::GpuResourceView& gpuResourceView);

    private:
        wgpu::TextureView view;
    };
}