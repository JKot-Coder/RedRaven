#include "DeviceImpl.hpp"
#include "SwapChainImpl.hpp"
#include "GpuResourceImpl.hpp"
#include "GpuResourceViewImpl.hpp"
#include "CommandQueueImpl.hpp"
#include "CommandContextImpl.hpp"
#include "ShaderImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "FramebufferImpl.hpp"

#define ASSERT_IS_DEVICE_INITED ASSERT(inited)
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
    DeviceImpl::~DeviceImpl()
    {
        if (!inited)
            return;
    }

    bool DeviceImpl::Init(const DeviceDescription& description)
    {
        ASSERT(!inited);

        int m_ValidationLevel = -1;
        this->description = description;

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
        engineFactory.Attach(pFactoryD3D12);

        DL::EngineD3D12CreateInfo EngineCI;
        EngineCI.GraphicsAPIVersion = {11, 0};
        if (m_ValidationLevel >= 0)
            EngineCI.SetValidationLevel(static_cast<DL::VALIDATION_LEVEL>(m_ValidationLevel));

        EngineCI.AdapterId = DL::DEFAULT_ADAPTER_ID;
        EngineCI.NumImmediateContexts = 0;
        EngineCI.NumDeferredContexts = 1;

        eastl::vector<DL::IDeviceContext*> ppContexts;
        ppContexts.resize(RR::Max(1u, EngineCI.NumImmediateContexts) + EngineCI.NumDeferredContexts);
        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &device, ppContexts.data());

        if (!device)
        {
            Log::Format::Error("Unable to initialize Diligent Engine in Direct3D12 mode. The API may not be available, "
                               "or required features may not be supported by this GPU/driver/OS version.");
            return false;
        }

        immediateContext.Attach(ppContexts[0]);
        deferredContext.Attach(ppContexts[1]);

        inited = true;
        deviceType = DL::RENDER_DEVICE_TYPE_D3D12;
        return true;
#endif
        return false;
    }

    void DeviceImpl::Present(SwapChain* swapChain)
    {
        ASSERT_IS_DEVICE_INITED;
        ASSERT(swapChain);

        ASSERT(dynamic_cast<SwapChainImpl*>(swapChain->GetPrivateImpl()));
        auto swapChainImpl = static_cast<SwapChainImpl*>(swapChain->GetPrivateImpl());

        immediateContext->Flush();

        // Diligent Engine waits for gpu on Present()
        swapChainImpl->Present();
    }

    void DeviceImpl::MoveToNextFrame(uint64_t frameIndex)
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(frameIndex);

        // Nothing to do here. Diligent Engine handles frame waiting on swapChain->Preset();
        // SEE swapChain->SetMaximumFrameLatency
    }

    GpuResourceFootprint DeviceImpl::GetResourceFootprint(const GpuResourceDescription& description) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(description);
        NOT_IMPLEMENTED();
        return {};
    }

    void DeviceImpl::Compile(CommandList2& commandList)
    {
        ASSERT_IS_DEVICE_INITED;


        ASSERT(dynamic_cast<CommandListImpl*>(commandList.GetPrivateImpl()));
        auto commandListImpl = static_cast<CommandListImpl*>(commandList.GetPrivateImpl());

        // MUTEX HERE AND DEFFERED CONTEXT
        commandListImpl->Compile(commandList, deferredContext);
    }

    void DeviceImpl::InitBuffer(Buffer& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandList(CommandList& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandList2(CommandList2& resource) const
    {
        ASSERT_IS_DEVICE_INITED;
        UNUSED(resource);

        auto impl = eastl::make_unique<CommandListImpl>();
        impl->Init();
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitCommandQueue(CommandQueue& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<CommandQueueImpl>();
        impl->Init(immediateContext);  // TODO pick based on queue type!
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitFence(Fence& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitFramebuffer(Framebuffer& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<FramebufferImpl>();
        impl->Init(resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitGpuResourceView(GpuResourceView& view) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto viewImpl = std::make_unique<GpuResourceViewImpl>();
        viewImpl->Init(view);
        view.SetPrivateImpl(viewImpl.release());
    }

    void DeviceImpl::InitSwapChain(SwapChain& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<SwapChainImpl>();
        impl->Init(deviceType, device, engineFactory, immediateContext, resource.GetDescription(), description.maxFramesInFlight);
        resource.SetPrivateImpl(impl.release());
    }

    // TODO ASSERT INITED everywhere
    void DeviceImpl::InitTexture(Texture& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = std::make_unique<GpuResourceImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitShader(Shader& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = std::make_unique<ShaderImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitPipelineState(PipelineState& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = std::make_unique<PipelineStateImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }
}