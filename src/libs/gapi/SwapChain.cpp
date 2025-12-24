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
            ASSERT(desc.bufferCount <= MAX_BACK_BUFFER_COUNT);
            ASSERT(desc.gpuResourceFormat != GpuResourceFormat::Unknown);
            ASSERT(desc.windowNativeHandle.has_value());
        }

        SwapChain::~SwapChain(){}

        void SwapChain::Resize(uint32_t width, uint32_t height)
        {
            ASSERT(width > 0);
            ASSERT(height > 0);

            desc_.width = width;
            desc_.height = height;

            eastl::array<Texture*, MAX_BACK_BUFFER_COUNT> backBuffers;
            for (uint32_t i = 0; i < desc_.bufferCount; i++)
                backBuffers[i] = backBuffers_[i].get();

            GetPrivateImpl()->Resize(width, height, backBuffers, depthBuffer_.get());

            for (auto& backBuffer : backBuffers_)
                backBuffer = nullptr;

            depthBuffer_.reset();
        }

        Texture::SharedPtr SwapChain::GetBackBufferTexture(uint32_t index)
        {
            ASSERT(index < desc_.bufferCount);

            if (backBuffers_[index])
                return backBuffers_[index];

            // TODO  description_.width = 0 sometimes happends
            const GpuResourceDesc desc = GpuResourceDesc::Texture2D(desc_.width, desc_.height, desc_.gpuResourceFormat, GpuResourceBindFlags::RenderTarget, GpuResourceUsage::Default, 1, 1);

            auto& deviceContext = Render::DeviceContext::Instance();

            backBuffers_[index] = deviceContext.CreateSwapChainBackBuffer(
                this,
                index,
                desc,
                fmt::sprintf("%s BackBufferTexture:%d", "SwapChain", index)); // TODO move it

            return backBuffers_[index];
        }

        Texture::SharedPtr SwapChain::GetDepthBufferTexture()
        {
            if (depthBuffer_)
                return depthBuffer_;

            const GpuResourceDesc desc = GpuResourceDesc::Texture2D(desc_.width, desc_.height, desc_.depthStencilFormat, GpuResourceBindFlags::DepthStencil, GpuResourceUsage::Default, 1, 1);

            auto& deviceContext = Render::DeviceContext::Instance();

            depthBuffer_ = deviceContext.CreateSwapChainDepthBuffer(this, desc);
            return depthBuffer_;
        }
    }
}