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

        GAPI::SwapChain* GetSwapChain();
        GAPI::Texture* GetBackBuffer();
        GAPI::RenderTargetView* GetBackBufferRTV();

    private:
        TextureUniquePtr backBuffer_;
        RenderTargetViewUniquePtr backBufferRTV_;
        SwapChainUniquePtr swapChain_;
    };
}