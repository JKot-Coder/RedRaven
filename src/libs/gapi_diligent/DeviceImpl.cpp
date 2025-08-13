#include "DeviceImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::Diligent
{
    DeviceImpl::DeviceImpl() { }
    DeviceImpl::~DeviceImpl() { }

    bool DeviceImpl::Init(const IDevice::Description& description)
    {
        UNUSED(description);
        NOT_IMPLEMENTED();
        return true;
    }

    void DeviceImpl::Present(const eastl::shared_ptr<SwapChain>& swapChain)
    {
        UNUSED(swapChain);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::MoveToNextFrame(uint64_t frameIndex)
    {
        UNUSED(frameIndex);
        NOT_IMPLEMENTED();
    }

    GpuResourceFootprint DeviceImpl::GetResourceFootprint(const GpuResourceDescription& description) const
    {
        UNUSED(description);
        NOT_IMPLEMENTED();
        return {};
    }

    void DeviceImpl::InitBuffer(const eastl::shared_ptr<Buffer>& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandList(CommandList& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandQueue(CommandQueue& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitFence(Fence& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitFramebuffer(Framebuffer& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitGpuResourceView(GpuResourceView& view) const
    {
        UNUSED(view);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitSwapChain(SwapChain& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitTexture(const eastl::shared_ptr<Texture>& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

}