#include "SwapChainImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            Result SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const U8String& name)
            {
                ASSERT(device);
                ASSERT(commandQueue);

                // Swapchain don't have name
                std::ignore = name;

         //      const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                ComSharedPtr<IDXGISwapChain1> swapChain1;
                // Create a swap chain for the window.
           /*     D3DCallMsg(dxgiFactory_->CreateSwapChainForHwnd(
                               commandQueue.get(),
                               presentOptions.windowHandle,
                               &targetSwapChainDesc,
                               nullptr,
                               nullptr,
                               swapChain1.put()),
                    "CreateSwapChainForHwnd");*/

                if (swapChain1.try_as(D3DSwapChain_))
                {
                    ASSERT(false);
                    return Result::Fail;
                }

                return Result::Ok;
            }
        }
    }
}