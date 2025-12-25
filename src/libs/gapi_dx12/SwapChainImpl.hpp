#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        class SwapChainImpl final : public ISwapChain
        {
        public:
            SwapChainImpl() = default;
            ~SwapChainImpl();

            void Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDesc& description);
            void UpdateCurrentBackBufferTexture(Texture& resource) const override;
            void Resize(uint32_t width, uint32_t height) override;

            virtual eastl::any GetWaitableObject() const override;

            HRESULT Present(uint32_t interval);

            const ComSharedPtr<IDXGISwapChain3>& GetD3DObject() const { return D3DSwapChain_; }

        private:
            ComSharedPtr<IDXGISwapChain3> D3DSwapChain_;
        };
    }
}