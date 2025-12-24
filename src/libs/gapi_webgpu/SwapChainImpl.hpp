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

        void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const override;
        void Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFERS_COUNT>& backBuffers) override;

        virtual eastl::any GetWaitableObject() const override;
        uint32_t GetCurrentBackBufferIndex() const override;

        void Present();

    private:
        wgpu::SurfaceConfiguration surfaceConfiguration;
        wgpu::Surface surface;
    };
}
