#include "SwapChain.hpp"

#include "gapi/SwapChain.hpp"
#include "render/DeviceContext.hpp"

namespace RR::Render
{
    SwapChain::~SwapChain() { }
    SwapChain::SwapChain(const GAPI::SwapChainDesc& desc)
    {
        swapChain_ = DeviceContext::Instance().CreateSwapchain(desc);
        resetBackBuffer();
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        swapChain_->Resize(width, height);
        resetBackBuffer();
    }

    void SwapChain::resetBackBuffer()
    {
        const GAPI::SwapChainDesc& swapChainDesc = swapChain_->GetDesc();
        const GAPI::GpuResourceDesc backBufferDesc = GAPI::GpuResourceDesc::Texture2D(swapChainDesc.width, swapChainDesc.height, swapChainDesc.backBufferFormat, GAPI::GpuResourceBindFlags::RenderTarget, GAPI::GpuResourceUsage::Default, 1, 1);
        backBuffer_ = DeviceContext::Instance().CreateSwapChainBackBuffer(*swapChain_.get(), backBufferDesc, "SwapChain BackBuffer");

        const GAPI::GpuResourceViewDesc rtvDesc = GAPI::GpuResourceViewDesc::Texture(swapChainDesc.backBufferFormat, 0, 1, 0, 1);
        backBufferRTV_ = DeviceContext::Instance().CreateRenderTargetView(*backBuffer_.get(), rtvDesc);
    }

    const GAPI::SwapChainDesc& SwapChain::GetDesc() const { return swapChain_->GetDesc(); }

    void SwapChain::UpdateBackBuffer()
    {
        swapChain_->UpdateBackBuffer(*backBuffer_, *backBufferRTV_);
    }
}