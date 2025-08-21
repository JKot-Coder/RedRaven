#include "SwapChainImpl.hpp"

#if D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif

#include "gapi_diligent/GpuResourceImpl.hpp"
#include "gapi_diligent/GpuResourceViewImpl.hpp"
#include "gapi_diligent/Utils.hpp"

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
        diligentSwapChainDesc.ColorBufferFormat = GetDLTextureFormat(description.gpuResourceFormat);
        diligentSwapChainDesc.DepthBufferFormat = GetDLTextureFormat(description.depthStencilFormat);
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

        backBufferCount = swapChainInitDesc.BufferCount;

        rtvs.reserve(swapChainInitDesc.BufferCount);
        resetRTVs();

        swapChain->SetMaximumFrameLatency(frameLatency);
    }

    void SwapChainImpl::resetRTVs()
    {
        rtvs.clear();
        for (uint32_t i = 0; i < backBufferCount; i++)
        {
            auto* rtv = swapChain->GetCurrentBackBufferRTV();
            ASSERT(rtv);

            // Due horoble impelemntation of swapchain in Diligent we need a hacky way
            // to get all RTV from the swapchain
            rtvs.push_back(rtv);

            swapChain->Present(0);
        }
        ASSERT(rtvs[0] == swapChain->GetCurrentBackBufferRTV());
    }

    void SwapChainImpl::Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFER_COUNT>& backBuffers)
    {
        ASSERT(swapChain);

        for (uint32_t i = 0; i < backBufferCount; i++)
        {
            auto* backBuffer = backBuffers[i];
            if (!backBuffer)
                continue;

            // Const cast is bad, but Diligent swapchain API is worse
            auto* rtv = const_cast<RenderTargetView*>(backBuffer->GetRTV());
            static_cast<GpuResourceViewImpl*>(rtv->GetPrivateImpl())->DestroyResource();
            rtv->SetPrivateImpl(nullptr);

            static_cast<GpuResourceImpl*>(backBuffer->GetPrivateImpl())->DestroyResource();
            backBuffer->SetPrivateImpl(nullptr);
        }

        swapChain->Resize(width, height);
        resetRTVs();
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