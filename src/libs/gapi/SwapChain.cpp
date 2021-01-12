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
            //TODO fix it
            ASSERT(description.windowHandle != 0);
        }

        Result SwapChain::Reset(const SwapChainDescription& description)
        {
            ASSERT(description.isStereo == description_.isStereo);
            ASSERT(description.bufferCount == description_.bufferCount);
            ASSERT(description.gpuResourceFormat == description_.gpuResourceFormat);
            ASSERT(description.windowHandle == description_.windowHandle);

            Result result = GetPrivateImpl()->Reset(description, backBuffers_);

            if (result == Result::Ok)
            {
                description_ = description;

                for (auto& backBuffer : backBuffers_)
                {
                    //TODO assert
                    backBuffer = nullptr;
                }
            }

            return result;
        }

        Texture::SharedPtr SwapChain::GetTexture(uint32_t backBufferIndex)
        {
            ASSERT(backBufferIndex < description_.bufferCount);

            if (backBuffers_[backBufferIndex])
                return backBuffers_[backBufferIndex];

            const TextureDescription desc = TextureDescription::Create2D(1, 1, description_.gpuResourceFormat);
            auto& renderContext = Render::RenderContext::Instance();

            backBuffers_[backBufferIndex] = renderContext.CreateSwapChainBackBuffer(
                std::static_pointer_cast<SwapChain>(shared_from_this()),
                backBufferIndex,
                desc,
                GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::ShaderResource,
                fmt::sprintf("%s BackBufferTexture:%d", name_, backBufferIndex)); //TODO move it

            return backBuffers_[backBufferIndex];
        }
    }
}