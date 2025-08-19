#include "SwapChainImpl.hpp"

#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

#include "platform/Window.hpp"
#include <Windows.h>

namespace RR
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

            SwapChainImpl::~SwapChainImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DSwapChain_);
            }

            void SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDescription& description)
            {
                UNUSED(device); // Not used
                ASSERT(dxgiFactory);
                ASSERT(commandQueue);
                ASSERT(CheckSwapchainDescription(description));

                // TODO move here?
                const auto& targetSwapChainDesc = D3DUtils::GetDxgiSwapChainDesc1(description, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                ComSharedPtr<IDXGISwapChain1> swapChain1;
                // Create a swap chain for the window.
                D3DCall(dxgiFactory->CreateSwapChainForHwnd(
                    commandQueue.get(),
                    std::any_cast<HWND>(description.window->GetNativeHandleRaw()),
                    &targetSwapChainDesc,
                    nullptr,
                    nullptr,
                    swapChain1.put()));

                if (FAILED(swapChain1.try_as<IDXGISwapChain3>(D3DSwapChain_)))
                    LOG_FATAL("Failed to cast swapchain");
                // TODO DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
                D3DSwapChain_->SetMaximumFrameLatency(description.bufferCount);
            }

            void SwapChainImpl::Reset(const SwapChainDescription& description, const Texture** backBuffers)
            {
                ASSERT(D3DSwapChain_);
                ASSERT(CheckSwapchainDescription(description));

                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCall(D3DSwapChain_->GetDesc1(&currentSwapChainDesc));

                const auto& targetSwapChainDesc = D3DUtils::GetDxgiSwapChainDesc1(description, DXGI_SWAP_EFFECT_FLIP_DISCARD);
                const auto swapChainCompatable = D3DUtils::SwapChainDesc1MatchesForReset(currentSwapChainDesc, targetSwapChainDesc);

                if (!swapChainCompatable)
                    LOG_FATAL("SwapChains incompatible");

                DeviceContext().GetGraphicsCommandQueue()->WaitForGpu();
                for (const auto& backBuffer : backBuffers)
                {
                    if (!backBuffer)
                        continue;

                    // Need to release d3d resource immediatly before ResizeBufferCall
                    backBuffer->GetPrivateImpl()->DestroyImmediatly();
                }

                HRESULT hr = D3DSwapChain_->ResizeBuffers(
                    targetSwapChainDesc.BufferCount,
                    targetSwapChainDesc.Width,
                    targetSwapChainDesc.Height,
                    targetSwapChainDesc.Format,
                    targetSwapChainDesc.Flags);

                if (FAILED(hr))
                {
                    LOG_FATAL("Can't resize swapchain buffers. Error: {}", D3DUtils::HResultToString(hr));
                }

                ASSERT(SUCCEEDED(hr)); // TODO

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                //   if (voidU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                // {
                // }*/
            }

            uint32_t SwapChainImpl::GetCurrentBackBufferIndex() const
            {
                ASSERT(D3DSwapChain_);

                return D3DSwapChain_->GetCurrentBackBufferIndex();
            }

            std::any SwapChainImpl::GetWaitableObject() const
            {
                ASSERT(D3DSwapChain_);

                return D3DSwapChain_->GetFrameLatencyWaitableObject();
            }

            void SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource)
            {
                ASSERT(resource);
                ASSERT(!resource->GetPrivateImpl());
                ASSERT(D3DSwapChain_);
                ASSERT(resource->GetDescription().usage == GpuResourceUsage::Default);
                ASSERT(IsSet(resource->GetDescription().bindFlags, GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::ShaderResource));

#ifdef ENABLE_ASSERTS
                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCall(D3DSwapChain_->GetDesc1(&currentSwapChainDesc));
                ASSERT(backBufferIndex <= currentSwapChainDesc.BufferCount);
#endif

                ComSharedPtr<ID3D12Resource> backBuffer;
                D3DCall(D3DSwapChain_->GetBuffer(backBufferIndex, IID_PPV_ARGS(backBuffer.put())));
                ASSERT(backBuffer);

                auto impl = new ResourceImpl();
                impl->Init(backBuffer, nullptr, resource->GetName());
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