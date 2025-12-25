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

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                bool CheckSwapchainDescription(const SwapChainDesc& desc)
                {
                    return (desc.backBuffersCount > 0 && desc.backBuffersCount <= MAX_BACK_BUFFERS_COUNT);
                }
            }

            SwapChainImpl::~SwapChainImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DSwapChain_);
            }

            void SwapChainImpl::Init(const ComSharedPtr<ID3D12Device>& device, const ComSharedPtr<IDXGIFactory2>& dxgiFactory, const ComSharedPtr<ID3D12CommandQueue>& commandQueue, const SwapChainDesc& description)
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
                    eastl::any_cast<HWND>(description.windowNativeHandle),
                    &targetSwapChainDesc,
                    nullptr,
                    nullptr,
                    swapChain1.put()));

                if (FAILED(swapChain1.try_as<IDXGISwapChain3>(D3DSwapChain_)))
                    LOG_FATAL("Failed to cast swapchain");
                // TODO DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
                D3DSwapChain_->SetMaximumFrameLatency(description.backBuffersCount);
            }

            void SwapChainImpl::UpdateCurrentBackBufferTexture(Texture& resource) const
            {
                ASSERT(D3DSwapChain_);

                ComSharedPtr<ID3D12Resource> backBuffer;
                const auto backBufferIndex = D3DSwapChain_->GetCurrentBackBufferIndex();
                D3DCall(D3DSwapChain_->GetBuffer(backBufferIndex, IID_PPV_ARGS(backBuffer.put())));
                ASSERT(backBuffer);

                if(!resource.GetPrivateImpl())
                {
                    auto impl = new ResourceImpl();
                    impl->Init(backBuffer, nullptr, resource.GetName());
                    resource.SetPrivateImpl(impl);
                }

                resource.GetPrivateImpl<ResourceImpl>()->UpdateTextureResource(backBuffer);

                ASSERT(resource.GetDesc().usage == GpuResourceUsage::Default);
                ASSERT(IsSet(resource.GetDesc().bindFlags, GpuResourceBindFlags::RenderTarget));


                /*   ASSERT(D3DSwapChain_);


                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCall(D3DSwapChain_->GetDesc1(&currentSwapChainDesc));


                DeviceContext().GetGraphicsCommandQueue()->WaitForGpu();
                for (const auto& backBuffer : backBuffers)
                {
                    if (!backBuffer)
                        continue;

                    // Need to release d3d resource immediatly before ResizeBufferCall
                    backBuffer->GetPrivateImpl()->DestroyImmediatly();
                }


                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                //   if (voidU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                // {
                // }*/
            }

            eastl::any SwapChainImpl::GetWaitableObject() const
            {
                ASSERT(D3DSwapChain_);

                return D3DSwapChain_->GetFrameLatencyWaitableObject();
            }

            void SwapChainImpl::Resize(uint32_t width, uint32_t height)
            {
                ASSERT(width > 0);
                ASSERT(height > 0);

                DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
                D3DSwapChain_->GetDesc1(&swapChainDesc);
                swapChainDesc.Width = width;
                swapChainDesc.Height = height;

                HRESULT hr = D3DSwapChain_->ResizeBuffers(
                    swapChainDesc.BufferCount,
                    swapChainDesc.Width,
                    swapChainDesc.Height,
                    swapChainDesc.Format,
                    swapChainDesc.Flags);

                if (FAILED(hr))
                {
                    LOG_FATAL("Can't resize swapchain buffers. Error: {}", D3DUtils::HResultToString(hr));
                }

                ASSERT(SUCCEEDED(hr)); // TODO
            }

            HRESULT SwapChainImpl::Present()
            {
                ASSERT(D3DSwapChain_);

                DXGI_PRESENT_PARAMETERS params = {};
                return D3DSwapChain_->Present1(0, DXGI_PRESENT_ALLOW_TEARING, &params);
            }
        }
    }
}