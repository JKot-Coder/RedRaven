#include "SwapChain.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

namespace RR
{
    namespace GAPI
    {
        SwapChain::SwapChain(const SwapChainDescription& description)
            : Resource(Type::SwapChain),
              description_(description)
        {
            ASSERT(description.width > 0);
            ASSERT(description.height > 0);
            ASSERT(description.isStereo == false);
            ASSERT(description.bufferCount <= MAX_BACK_BUFFER_COUNT);
            ASSERT(description.gpuResourceFormat != GpuResourceFormat::Unknown);
            ASSERT(description.windowNativeHandle.has_value());
        }

        SwapChain::~SwapChain(){}

        void SwapChain::Resize(uint32_t width, uint32_t height)
        {
            ASSERT(width > 0);
            ASSERT(height > 0);

            description_.width = width;
            description_.height = height;

            eastl::array<Texture*, MAX_BACK_BUFFER_COUNT> backBuffers;
            for (uint32_t i = 0; i < description_.bufferCount; i++)
                backBuffers[i] = backBuffers_[i].get();

            GetPrivateImpl()->Resize(width, height, backBuffers);

            for (auto& backBuffer : backBuffers_)
                backBuffer = nullptr;
        }

        Texture* SwapChain::GetBackBufferTexture(uint32_t index)
        {
            ASSERT(index < description_.bufferCount);

            if (backBuffers_[index])
                return backBuffers_[index].get();

            // TODO  description_.width = 0 sometimes happends
            const GpuResourceDescription desc = GpuResourceDescription::Texture2D(description_.width, description_.height, description_.gpuResourceFormat, GpuResourceBindFlags::RenderTarget, GpuResourceUsage::Default, 1, 1);

            auto& deviceContext = Render::DeviceContext::Instance();

            backBuffers_[index] = deviceContext.CreateSwapChainBackBuffer(
                this,
                index,
                desc,
                fmt::sprintf("%s BackBufferTexture:%d", "SwapChain", index)); // TODO move it

            return backBuffers_[index].get();
        }
    }
}