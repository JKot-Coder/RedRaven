#include "SwapChainImpl.hpp"

#if D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif

#include "gapi/GpuResource.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::Diligent
{
    SwapChainImpl::~SwapChainImpl() { }

    DL::SwapChainDesc getSwapChainInitDesc(const SwapChainDescription& description)
    {
        DL::SwapChainDesc diligentSwapChainDesc;
        diligentSwapChainDesc.BufferCount = description.bufferCount;
        diligentSwapChainDesc.Width = description.width;
        diligentSwapChainDesc.Height = description.height;
        // TODO: convert GpuResourceFormat to TEXTURE_FORMAT
        ASSERT(description.gpuResourceFormat == GpuResourceFormat::RGBA8UnormSrgb);
        diligentSwapChainDesc.ColorBufferFormat = DL::TEX_FORMAT_RGBA8_UNORM_SRGB;
        diligentSwapChainDesc.DepthBufferFormat = DL::TEX_FORMAT_UNKNOWN;
        diligentSwapChainDesc.DefaultDepthValue = 0.f;
        return diligentSwapChainDesc;
    }

    void SwapChainImpl::Init(DL::RENDER_DEVICE_TYPE deviceType,
                             const DL::RefCntAutoPtr<DL::IRenderDevice>& device,
                             const DL::RefCntAutoPtr<DL::IEngineFactory>& engineFactory,
                             DL::IDeviceContext* immediateContext,
                             const SwapChainDescription& description, const std::string& name)
    {

        UNUSED(name);


        DL::SwapChainDesc swapChainInitDesc = getSwapChainInitDesc(description);
#if OS_WINDOWS
        DL::Win32NativeWindow nativeWindow {eastl::any_cast<HWND>(description.windowNativeHandle)};
#elif
        static_assert(false, "Unsupported platform");
#endif

        switch (deviceType)
        {
#if D3D12_SUPPORTED
            case DL::RENDER_DEVICE_TYPE_D3D12:
            {
                DL::IEngineFactoryD3D12* factoryD3D12;
                engineFactory->QueryInterface(DL::IID_EngineFactoryD3D12, (DL::IObject**)&factoryD3D12);
                factoryD3D12->CreateSwapChainD3D12(device, immediateContext, swapChainInitDesc, DL::FullScreenModeDesc {}, nativeWindow, &swapChain);
            }
            break;
#endif
            default:
                ASSERT_MSG(false, "Unsupported device type");
        }
    }

    void SwapChainImpl::Reset(const SwapChainDescription& description, const std::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers)
    {
        UNUSED(description);
        UNUSED(backBuffers);
        NOT_IMPLEMENTED();
    }

    eastl::any SwapChainImpl::GetWaitableObject() const
    {
        NOT_IMPLEMENTED();
        return {};
    }

    uint32_t SwapChainImpl::GetCurrentBackBufferIndex() const
    {
        NOT_IMPLEMENTED();
        return 0;
    }

    void SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, const eastl::shared_ptr<Texture>& resource)
    {
        UNUSED(backBufferIndex);
        UNUSED(resource);
        NOT_IMPLEMENTED();
    }
}