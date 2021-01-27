#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class SwapChainImpl final : public ISwapChain
            {
            public:
                SwapChainImpl() = default;

                void ReleaseD3DObjects();

                void Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name);
                void Reset(const SwapChainDescription& description, const std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers);

                void InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource) override;

                HRESULT Present(uint32_t interval);

                const ComSharedPtr<IDXGISwapChain3>& GetD3DObject() const { return D3DSwapChain_; }

            private:
                ComSharedPtr<IDXGISwapChain3> D3DSwapChain_;
            };
        }
    }
}