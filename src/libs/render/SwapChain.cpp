#include "SwapChain.hpp"

#include "gapi/SwapChain.hpp"
#include "render/DeviceContext.hpp"

namespace RR::Render
{
    SwapChain::~SwapChain() { }
    SwapChain::SwapChain(const GAPI::SwapChainDesc& desc)
    {
        swapChain_ = DeviceContext::Instance().CreateSwapchain(desc);

        const GAPI::SwapChainDesc& swapChainDesc = swapChain_->GetDesc();
        const GAPI::GpuResourceDesc backBufferDesc = GAPI::GpuResourceDesc::Texture2D(swapChainDesc.width, swapChainDesc.height, swapChainDesc.backBufferFormat, GAPI::GpuResourceBindFlags::RenderTarget, GAPI::GpuResourceUsage::Default, 1, 1);
        backBuffer_ = DeviceContext::Instance().CreateSwapChainBackBuffer(*swapChain_.get(), backBufferDesc, "SwapChain BackBuffer");

        const GAPI::GpuResourceViewDesc rtvDesc = GAPI::GpuResourceViewDesc::Texture(swapChainDesc.backBufferFormat, 0, 1, 0, 1);
        backBufferRTV_ = DeviceContext::Instance().CreateRenderTargetView(*backBuffer_.get(), rtvDesc);
    }

    const GAPI::SwapChainDesc& SwapChain::GetDesc() const { return swapChain_->GetDesc(); }

    GAPI::SwapChain* SwapChain::GetSwapChain() { return swapChain_.get(); }

    GAPI::Texture* SwapChain::GetBackBuffer()
    {
        // TODO not update every time
        swapChain_->UpdateBackBufferTexture(*backBuffer_, *backBufferRTV_);

        return backBuffer_.get();
    }

    GAPI::RenderTargetView* SwapChain::GetBackBufferRTV()
    {
        return backBufferRTV_.get();
    }
}