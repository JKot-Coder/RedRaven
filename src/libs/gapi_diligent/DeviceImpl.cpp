#include "DeviceImpl.hpp"
#include "SwapChainImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "RenderDevice.h"

#if D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif

#include "gapi_diligent/Device.hpp"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    DeviceImpl::DeviceImpl() { }
    DeviceImpl::~DeviceImpl() { }

    bool DeviceImpl::Init(const DeviceDescription& description)
    {
        int m_ValidationLevel = -1;

        // Todo support other APIs
#if D3D12_SUPPORTED
#if ENGINE_DLL
        // Load the dll and import GetEngineFactoryD3D12() function
        DL::GetEngineFactoryD3D12Type GetEngineFactoryD3D12 = DL::LoadGraphicsEngineD3D12();
        if (!GetEngineFactoryD3D12)
        {
            Log::Format::Error("Failed to load GraphicsEngineD3D12");
            return false;
        }
#endif
        DL::IEngineFactoryD3D12* pFactoryD3D12 = GetEngineFactoryD3D12();
        if (!pFactoryD3D12->LoadD3D12())
        {
            Log::Format::Error("Failed to load Direct3D12");
            return false;
        }
        engineFactory = static_cast<DL::IEngineFactory*>(pFactoryD3D12);

        DL::EngineD3D12CreateInfo EngineCI;
        EngineCI.GraphicsAPIVersion = {11, 0};
        if (m_ValidationLevel >= 0)
            EngineCI.SetValidationLevel(static_cast<DL::VALIDATION_LEVEL>(m_ValidationLevel));

        EngineCI.AdapterId = DL::DEFAULT_ADAPTER_ID;
        EngineCI.NumImmediateContexts = 0;
        EngineCI.NumDeferredContexts = 0;

        eastl::vector<DL::IDeviceContext*> ppContexts;
        ppContexts.resize(RR::Max(1u, EngineCI.NumImmediateContexts) + EngineCI.NumDeferredContexts);
        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &device, ppContexts.data());

        if (!device)
        {
            Log::Format::Error("Unable to initialize Diligent Engine in Direct3D12 mode. The API may not be available, "
                               "or required features may not be supported by this GPU/driver/OS version.");
            return false;
        }

        immediateContext = ppContexts[0];
        inited = true;
        deviceType = DL::RENDER_DEVICE_TYPE_D3D12;
        return true;
#endif
        return false;
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
        auto impl = eastl::make_unique<SwapChainImpl>();
        impl->Init(deviceType, device, engineFactory, immediateContext, resource.GetDescription(), resource.GetName());
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitTexture(const eastl::shared_ptr<Texture>& resource) const
    {
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

}