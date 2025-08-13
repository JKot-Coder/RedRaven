#pragma once

#include "gapi/Device.hpp"

namespace RR::GAPI::Diligent
{
    class DeviceImpl final : public IDevice
    {
    public:
        DeviceImpl();
        ~DeviceImpl() override;

        bool Init(const IDevice::Description& description);
        void Present(const eastl::shared_ptr<SwapChain>& swapChain) override;
        void MoveToNextFrame(uint64_t frameIndex) override;

        GpuResourceFootprint GetResourceFootprint(const GpuResourceDescription& description) const override;

        void InitBuffer(const eastl::shared_ptr<Buffer>& resource) const override;
        void InitCommandList(CommandList& resource) const override;
        void InitCommandQueue(CommandQueue& resource) const override;
        void InitFence(Fence& resource) const override;
        void InitFramebuffer(Framebuffer& resource) const override;
        void InitGpuResourceView(GpuResourceView& view) const override;
        void InitSwapChain(SwapChain& resource) const override;
        void InitTexture(const eastl::shared_ptr<Texture>& resource) const override;

        std::any GetRawDevice() const override { ASSERT_MSG(false, "Not implemented"); return nullptr; }
    };
}