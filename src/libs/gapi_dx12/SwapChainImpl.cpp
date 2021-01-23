#include "SwapChainImpl.hpp"

#include "gapi/Result.hpp"
#include "gapi/SwapChain.hpp"

#include "gapi_dx12/Device.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                bool CheckSwapchainDescription(const SwapChainDescription& desc)
                {
                    return (desc.width > 0) && (desc.height > 0) && (desc.bufferCount > 0 && desc.bufferCount <= MAX_BACK_BUFFER_COUNT) && (desc.windowHandle) && (desc.isStereo == false);
                }
            }

            void SwapChainImpl::ReleaseD3DObjects(ResourceReleaseContext& releaseContext)
            {
                releaseContext.DeferredD3DResourceRelease(D3DSwapChain_);
            }

            Result SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name)
            {
                ASSERT(device)
                ASSERT(dxgiFactory)
                ASSERT(commandQueue)
                ASSERT(CheckSwapchainDescription(description));

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

                if (!swapChain1.try_as(D3DSwapChain_))
                {
                    ASSERT_MSG(false, "Failed to cast swapchain")
                    return Result::Fail;
                }

                swapChain1.as(D3DSwapChain_);

                return Result::Ok;
            }

            Result SwapChainImpl::Reset(const SwapChainDescription& description, const std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers)
            {
                ASSERT(D3DSwapChain_)
                ASSERT(CheckSwapchainDescription(description))

                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCallMsg(D3DSwapChain_->GetDesc1(&currentSwapChainDesc), "GetDesc1");

                const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(description, DXGI_SWAP_EFFECT_FLIP_DISCARD);
                const auto swapChainCompatable = D3DUtils::SwapChainDesc1MatchesForReset(currentSwapChainDesc, targetSwapChainDesc);

                if (!swapChainCompatable)
                {
                    LOG_ERROR("SwapChains incompatible");
                    return Result::Fail;
                }

                // Clear api references
                for (const auto& backBuffer : backBuffers)
                {
                    if (!backBuffer)
                        continue;

                    backBuffer->SetPrivateImpl(nullptr);
                }

                Log::Print::Info("W:%d,H:%d\n", targetSwapChainDesc.Width,
                                 targetSwapChainDesc.Height);

                HRESULT hr = D3DSwapChain_->ResizeBuffers(
                    targetSwapChainDesc.BufferCount,
                    targetSwapChainDesc.Width,
                    targetSwapChainDesc.Height,
                    targetSwapChainDesc.Format,
                    targetSwapChainDesc.Flags);

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                //   if (ResultU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                // {
                // }*/

                return Result::Ok;
            }

            Result SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource)
            {
                ASSERT(resource);
                ASSERT(!resource->GetPrivateImpl());
                ASSERT(D3DSwapChain_);

#ifdef ENABLE_ASSERTS
                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCallMsg(D3DSwapChain_->GetDesc1(&currentSwapChainDesc), "GetDesc1");
                ASSERT(backBufferIndex <= currentSwapChainDesc.BufferCount);
#endif

                ComSharedPtr<ID3D12Resource> backBuffer_;
                D3DCallMsg(D3DSwapChain_->GetBuffer(backBufferIndex, IID_PPV_ARGS(backBuffer_.put())), "GetBuffer");
                ASSERT(backBuffer_);

                auto impl = new ResourceImpl();
                D3DCall(impl->Init(backBuffer_, resource->GetDescription(), resource->GetBindFlags(), resource->GetName()));
                resource->SetPrivateImpl(impl);

                return Result::Ok;
            }

            Result SwapChainImpl::Present(uint32_t interval)
            {
                ASSERT(D3DSwapChain_);

                DXGI_PRESENT_PARAMETERS params = {};
                D3DCallMsg(D3DSwapChain_->Present1(interval, 0, &params), "Present1");

                return Result::Ok;
            }
        }
    }
}