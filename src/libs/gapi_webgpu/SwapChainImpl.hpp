#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class SwapChainImpl final : public ISwapChain
    {
    public:
        SwapChainImpl() = default;
        ~SwapChainImpl();

        void Init(const wgpu::Instance& instance, const wgpu::Device& device, const GAPI::SwapChainDesc& desc);

        void UpdateBackBuffer(Texture& resource, RenderTargetView& rtv) const override;
        void Resize(uint32_t width, uint32_t height) override;

        virtual eastl::any GetWaitableObject() const override;

        void Present();
        wgpu::SurfaceTexture GetSurfaceTexture() const { return surfaceTexture; }

    private:
        wgpu::SurfaceConfiguration surfaceConfiguration;
        wgpu::Surface surface;
        wgpu::SurfaceTexture surfaceTexture;
    };
}
