#pragma once

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
                Result Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const U8String& name);

                const ComSharedPtr<IDXGISwapChain3>& getD3DObject() const { return D3DSwapChain_; }

            private:
                ComSharedPtr<IDXGISwapChain3> D3DSwapChain_;
            };
        }
    }
}