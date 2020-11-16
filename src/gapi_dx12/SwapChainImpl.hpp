#pragma once

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class SwapChainImpl final
            {

            public:
                SwapChainImpl() = default;

                Result Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name);
                Result Reset(const SwapChainDescription& description);

                const ComSharedPtr<IDXGISwapChain3>& getD3DObject() const { return D3DSwapChain_; }

            private:
                ComSharedPtr<IDXGISwapChain3> D3DSwapChain_;
            };
        }
    }
}