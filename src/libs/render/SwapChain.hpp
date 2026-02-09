#pragma once

#include "gapi/ForwardDeclarations.hpp"

namespace RR::Render
{
    class SwapChain
    {
    public:
        ~SwapChain();
        SwapChain(const GAPI::SwapChainDesc& desc);

        const GAPI::SwapChainDesc& GetDesc() const;

        GAPI::SwapChain* GetSwapChain() { return swapChain_.get(); }
        GAPI::Texture* GetBackBuffer() { return backBuffer_.get(); }
        GAPI::RenderTargetView* GetBackBufferRTV() { return backBufferRTV_.get(); }

    private:
        void Resize(uint32_t width, uint32_t height);
        void UpdateBackBuffer();
        void resetBackBuffer();

        friend class DeviceContext;

    private:
        eastl::unique_ptr<GAPI::Texture> backBuffer_;
        eastl::unique_ptr<GAPI::RenderTargetView> backBufferRTV_;
        eastl::unique_ptr<GAPI::SwapChain> swapChain_;
    };
}