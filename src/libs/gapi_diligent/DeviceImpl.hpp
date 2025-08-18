#pragma once

#include "gapi/Device.hpp"

#include "RefCntAutoPtr.hpp"
#include "GraphicsTypes.h"

namespace DL = ::Diligent;

namespace Diligent
{
    class IDeviceContext;
    class IRenderDevice;
    class IEngineFactory;
}

namespace RR::GAPI::Diligent
{
    class DeviceImpl final : public IDevice
    {
    public:
        DeviceImpl();
        ~DeviceImpl() override;

        bool Init(const GAPI::DeviceDescription& description);
        void Present(SwapChain* swapChain) override;
        void MoveToNextFrame(uint64_t frameIndex) override;

        GpuResourceFootprint GetResourceFootprint(const GpuResourceDescription& description) const override;

        void InitBuffer(Buffer& resource) const override;
        void InitCommandList(CommandList& resource) const override;
        void InitCommandContext(CommandContext& resource) const override;
        void InitCommandQueue(CommandQueue& resource) const override;
        void InitFence(Fence& resource) const override;
        void InitFramebuffer(Framebuffer& resource) const override;
        void InitGpuResourceView(GpuResourceView& view) const override;
        void InitSwapChain(SwapChain& resource) const override;
        void InitTexture(Texture& resource) const override;

        std::any GetRawDevice() const override { ASSERT_MSG(false, "Not implemented"); return nullptr; }

    private:
        bool inited = false;
        GAPI::DeviceDescription description = {};
        DL::RENDER_DEVICE_TYPE deviceType = DL::RENDER_DEVICE_TYPE_UNDEFINED;
        DL::RefCntAutoPtr<DL::IDeviceContext> immediateContext;
        DL::RefCntAutoPtr<DL::IRenderDevice> device;
        DL::RefCntAutoPtr<DL::IEngineFactory> engineFactory;
    };
}