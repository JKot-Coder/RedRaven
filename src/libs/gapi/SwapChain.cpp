#include "SwapChain.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

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
        }
    }
}