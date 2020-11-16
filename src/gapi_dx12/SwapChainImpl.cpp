#include "SwapChainImpl.hpp"

#include "gapi/SwapChain.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            Result SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name)
            {
                ASSERT(device)
                ASSERT(dxgiFactory)
                ASSERT(commandQueue)

                // Swapchain don't have name
                std::ignore = name;

                const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(description, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                ComSharedPtr<IDXGISwapChain1> swapChain1;
                // Create a swap chain for the window.
                D3DCallMsg(dxgiFactory->CreateSwapChainForHwnd(
                               commandQueue.get(),
                               description.windowHandle,
                               &targetSwapChainDesc,
                               nullptr,
                               nullptr,
                               swapChain1.put()),
                    "CreateSwapChainForHwnd");

                if (swapChain1.try_as(D3DSwapChain_))
                {
                    ASSERT(false)
                    return Result::Fail;
                }

                return Result::Ok;
            }
        }
    }
}