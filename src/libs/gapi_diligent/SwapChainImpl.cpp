#include "SwapChainImpl.hpp"

#if D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif

#include "gapi_diligent/GpuResourceImpl.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

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
                             const SwapChainDescription& description, uint32_t frameLatency)
    {
        ASSERT(!swapChain);

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

        rtvs.reserve(swapChainInitDesc.BufferCount);
        for (uint32_t i = 0; i < swapChainInitDesc.BufferCount; i++)
        {
            auto* rtv = swapChain->GetCurrentBackBufferRTV();
            ASSERT(rtv);

            // Due horoble impelemntation of swapchain in Diligent we need a hacky way
            // to get all RTV from the swapchain
            rtvs.push_back(rtv);

            swapChain->Present(0);
        }
        ASSERT(rtvs[0] == swapChain->GetCurrentBackBufferRTV());

        swapChain->SetMaximumFrameLatency(frameLatency);
    }

    void SwapChainImpl::Reset(const SwapChainDescription& description, const Texture** backBuffers)
    {
        ASSERT(swapChain);
        UNUSED(description);
        UNUSED(backBuffers);
        NOT_IMPLEMENTED();
    }

    eastl::any SwapChainImpl::GetWaitableObject() const
    {
        ASSERT(swapChain);
        NOT_IMPLEMENTED();
        return {};
    }

    uint32_t SwapChainImpl::GetCurrentBackBufferIndex() const
    {
        ASSERT(swapChain);
        const auto* currentBackBufferRTV = swapChain->GetCurrentBackBufferRTV();

        for (uint32_t i = 0; i < rtvs.size(); i++)
        {
            if (rtvs[i] == currentBackBufferRTV)
                return i;
        }

        ASSERT_MSG(false, "Current back buffer index not found");
        return -1;
    }

    void SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const
    {
        ASSERT(!resource.GetPrivateImpl());
        ASSERT(swapChain);
        ASSERT(resource.GetDescription().usage == GpuResourceUsage::Default);
        ASSERT(IsSet(resource.GetDescription().bindFlags, GpuResourceBindFlags::RenderTarget));
        ASSERT(backBufferIndex < rtvs.size());

        auto* backBufferRtv = rtvs[backBufferIndex];
        ASSERT(backBufferRtv);

        // We want to own this texture, so add ref to it
        auto* backBufferTexture = backBufferRtv->GetTexture();
        backBufferTexture->AddRef();

        auto impl = new GpuResourceImpl();
        impl->Init(backBufferTexture, resource);
        resource.SetPrivateImpl(impl);
    }

    void SwapChainImpl::Present()
    {
        ASSERT(swapChain);
        swapChain->Present(0);
    }
}