#include "SwapChain.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

namespace RR
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

        // Todo comment about multhreading
        // Todo just new size and that's it.
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

        Texture::SharedPtr SwapChain::GetBackBufferTexture(uint32_t index)
        {
            ASSERT(index < description_.bufferCount);

            if (backBuffers_[index])
                return backBuffers_[index];

            // TODO  description_.width = 0 sometimes happends
            const GpuResourceDescription desc = GpuResourceDescription::Texture2D(description_.width, description_.height, description_.gpuResourceFormat, GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::ShaderResource, GpuResourceUsage::Default, 1, 1);

            auto& renderContext = Render::DeviceContext::Instance();

            backBuffers_[index] = renderContext.CreateSwapChainBackBuffer(
                std::static_pointer_cast<SwapChain>(shared_from_this()),
                index,
                desc,
                fmt::sprintf("%s BackBufferTexture:%d", GetName(), index)); //TODO move it

            return backBuffers_[index];
        }
    }
}