#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "render/ResourcePointers.hpp"

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

        void UpdateBackBuffer();

    private:
        TextureUniquePtr backBuffer_;
        RenderTargetViewUniquePtr backBufferRTV_;
        SwapChainUniquePtr swapChain_;
    };
}