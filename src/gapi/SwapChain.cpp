#include "SwapChain.hpp"

#include "gapi/Resource.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        SwapChain::SwapChain(const SwapChainDescription& description, const U8String& name)
            : InterfaceWrapObject(Object::Type::SwapChain, name),
              description_(description)
        {
            ASSERT(description.width > 0);
            ASSERT(description.height > 0);
            ASSERT(description.isStereo == false);
            ASSERT(description.bufferCount <= MAX_BACK_BUFFER_COUNT);
            ASSERT(description.resourceFormat != ResourceFormat::Unknown);
            //TODO fix it
            ASSERT(description.windowHandle != 0);
        }

        Result SwapChain::Reset(const SwapChainDescription& description)
        {
            ASSERT(description.isStereo == description_.isStereo);
            ASSERT(description.bufferCount == description_.bufferCount);
            ASSERT(description.resourceFormat == description_.resourceFormat);
            ASSERT(description.windowHandle == description_.windowHandle);

            Result result = GetInterface()->Reset(description, backBuffers_);

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

            const TextureDescription desc = TextureDescription::Create2D(1, 1, description_.resourceFormat);
            auto& renderContext = Render::RenderContext::Instance();

            backBuffers_[backBufferIndex] = renderContext.CreateSwapChainBackBuffer(
                std::static_pointer_cast<SwapChain>(shared_from_this()),
                backBufferIndex,
                desc,
                ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource,
                fmt::sprintf("%s BackBufferTexture:%d", name_, backBufferIndex)); //TODO move it

            return backBuffers_[backBufferIndex];
        }
    }
}