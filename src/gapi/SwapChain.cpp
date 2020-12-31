#include "SwapChain.hpp"

#include "gapi/Resource.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {

        SwapChain::SwapChain(const std::shared_ptr<CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name)
            : InterfaceWrapObject(Object::Type::SwapChain, name),
              description_(description),
              commandQueue_(commandQueue)
        {
            ASSERT(commandQueue);
            ASSERT(description.width > 0);
            ASSERT(description.height > 0);
            ASSERT(description.isStereo == false);
            ASSERT(description.bufferCount <= MAX_BACK_BUFFER_COUNT);
            ASSERT(description.resourceFormat != ResourceFormat::Unknown);
            //TODO fix it
            ASSERT(description.windowHandle != 0);
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
                Resource::BindFlags::RenderTarget | Resource::BindFlags::ShaderResource,
                fmt::sprintf("%s BackBufferTexture:%d", name_, backBufferIndex));

            return backBuffers_[backBufferIndex];
        }

    }
}