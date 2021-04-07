#include "SwapChain.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        SwapChain::SwapChain(const SwapChainDescription& description, const U8String& name)
            : Resource(Object::Type::SwapChain, name),
              description_(description)
        {
            ASSERT(description.width > 0);
            ASSERT(description.height > 0);
            ASSERT(description.isStereo == false);
            ASSERT(description.bufferCount <= MAX_BACK_BUFFER_COUNT);
            ASSERT(description.gpuResourceFormat != GpuResourceFormat::Unknown);
            ASSERT(description.window);
        }

        void SwapChain::Reset(const SwapChainDescription& description)
        {
            ASSERT(description.isStereo == description_.isStereo);
            ASSERT(description.bufferCount == description_.bufferCount);
            ASSERT(description.gpuResourceFormat == description_.gpuResourceFormat);
            ASSERT(description.window == description_.window);

            GetPrivateImpl()->Reset(description, backBuffers_);

            description_ = description;
            for (auto& backBuffer : backBuffers_)
            {
                //TODO assert
                backBuffer = nullptr;
            }
        }

        Texture::SharedPtr SwapChain::GetTexture(uint32_t backBufferIndex)
        {
            ASSERT(backBufferIndex < description_.bufferCount);

            if (backBuffers_[backBufferIndex])
                return backBuffers_[backBufferIndex];

            const TextureDescription desc = TextureDescription::Create2D(description_.width, description_.height, description_.gpuResourceFormat, GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::ShaderResource, 1, 1);
            auto& renderContext = Render::RenderContext::Instance();

            backBuffers_[backBufferIndex] = renderContext.CreateSwapChainBackBuffer(
                std::static_pointer_cast<SwapChain>(shared_from_this()),
                backBufferIndex,
                desc,
                fmt::sprintf("%s BackBufferTexture:%d", GetName(), backBufferIndex)); //TODO move it

            return backBuffers_[backBufferIndex];
        }
    }
}