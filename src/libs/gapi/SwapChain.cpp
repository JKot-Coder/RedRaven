#include "SwapChain.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

namespace RR
{
    namespace GAPI
    {
        SwapChain::SwapChain(const SwapChainDesc& desc)
            : Resource(Type::SwapChain),
              desc_(desc)
        {
            ASSERT(desc.width > 0);
            ASSERT(desc.height > 0);
            ASSERT(desc.backBuffersCount <= MAX_BACK_BUFFERS_COUNT);
            ASSERT(desc.backBufferFormat != GpuResourceFormat::Unknown);
            ASSERT(desc.windowNativeHandle.has_value());
        }

        SwapChain::~SwapChain(){}

        void SwapChain::Resize(uint32_t width, uint32_t height)
        {
            ASSERT(width > 0);
            ASSERT(height > 0);

            desc_.width = width;
            desc_.height = height;

            GetPrivateImpl()->Resize(width, height);
            backBuffer.reset();
        }

        Texture::SharedPtr SwapChain::GetCurrentBackBufferTexture()
        {
            if (!backBuffer)
            {
                // TODO  description_.width = 0 sometimes happends
                const GpuResourceDesc desc = GpuResourceDesc::Texture2D(desc_.width, desc_.height, desc_.backBufferFormat, GpuResourceBindFlags::RenderTarget, GpuResourceUsage::Default, 1, 1);
                auto& deviceContext = Render::DeviceContext::Instance();
                backBuffer = deviceContext.CreateSwapChainBackBuffer(
                    this,
                    desc,
                    fmt::sprintf("%s BackBufferTexture", "SwapChain")); // TODO move it
            }

            UpdateCurrentBackBufferTexture(*backBuffer);
            backBuffer->ReleaseRTV();

            return backBuffer;
        }
    }
}