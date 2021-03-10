#include "SwapChainImpl.hpp"

#include "gapi/SwapChain.hpp"

#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

#include <Windows.h>
#include "windowing/Window.hpp"

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
                    return (desc.bufferCount > 0 && desc.bufferCount <= MAX_BACK_BUFFER_COUNT) && (desc.window) && (desc.isStereo == false);
                }
            }

            void SwapChainImpl::ReleaseD3DObjects()
            {
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DSwapChain_);
            }

            void SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name)
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
                D3DCall(dxgiFactory->CreateSwapChainForHwnd(
                    commandQueue.get(),
                    description.window->GetNativeHandle(),
                    &targetSwapChainDesc,
                    nullptr,
                    nullptr,
                    swapChain1.put()));

                if (!swapChain1.try_as(D3DSwapChain_))
                    LOG_FATAL("Failed to cast swapchain");

                swapChain1.as(D3DSwapChain_);
            }

            void SwapChainImpl::Reset(const SwapChainDescription& description, const std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers)
            {
                ASSERT(D3DSwapChain_)
                ASSERT(CheckSwapchainDescription(description))

                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCall(D3DSwapChain_->GetDesc1(&currentSwapChainDesc));

                const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(description, DXGI_SWAP_EFFECT_FLIP_DISCARD);
                const auto swapChainCompatable = D3DUtils::SwapChainDesc1MatchesForReset(currentSwapChainDesc, targetSwapChainDesc);

                if (!swapChainCompatable)
                    LOG_FATAL("SwapChains incompatible");

                DeviceContext().GetGraphicsCommandQueue()->WaitForGpu();

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
                //   if (voidU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                // {
                // }*/
            }

            void SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource)
            {
                ASSERT(resource);
                ASSERT(!resource->GetPrivateImpl());
                ASSERT(D3DSwapChain_);
                ASSERT(resource->GetCpuAccess() == GpuResourceCpuAccess::None);
                ASSERT(IsSet(resource->GetBindFlags(), GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::ShaderResource));

#ifdef ENABLE_ASSERTS
                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCall(D3DSwapChain_->GetDesc1(&currentSwapChainDesc));
                ASSERT(backBufferIndex <= currentSwapChainDesc.BufferCount);
#endif

                ComSharedPtr<ID3D12Resource> backBuffer_;
                D3DCall(D3DSwapChain_->GetBuffer(backBufferIndex, IID_PPV_ARGS(backBuffer_.put())));
                ASSERT(backBuffer_);

                auto impl = new ResourceImpl();
                impl->Init(backBuffer_, nullptr, resource->GetName());
                resource->SetPrivateImpl(impl);
            }

            HRESULT SwapChainImpl::Present(uint32_t interval)
            {
                ASSERT(D3DSwapChain_);

                DXGI_PRESENT_PARAMETERS params = {};
                return D3DSwapChain_->Present1(interval, 0, &params);
            }
        }
    }
}