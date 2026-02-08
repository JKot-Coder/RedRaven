#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Texture.hpp"

// TODO temporary
#include <eastl/any.h>

namespace RR
{
    namespace Platform
    {
        class Window;
    }

    namespace Render
    {
        class SwapChain;
    }

    namespace GAPI
    {
        struct SwapChainDesc
        {
            enum class PresentMode : uint8_t
            {
                Fifo,       // VSync
                Mailbox,    // Low-latency VSync (if supported)
                Immediate   // No VSync (tearing)
            };

            uint32_t width = 0;
            uint32_t height = 0;

            PresentMode presentMode = PresentMode::Fifo;
            uint32_t backBuffersCount = 2;

            GpuResourceFormat backBufferFormat;

            eastl::any windowNativeHandle = nullptr;
        };

        class ISwapChain
        {
        public:
            virtual ~ISwapChain() {};

            virtual eastl::any GetWaitableObject() const = 0;

            virtual void UpdateBackBuffer(Texture& resource, RenderTargetView& rtv) const = 0;
            virtual void Resize(uint32_t width, uint32_t height) = 0;
        };

        class SwapChain final : public Resource<ISwapChain, false>
        {
        public:
            ~SwapChain();

            const SwapChainDesc& GetDesc() const { return desc_; }
            // TODO temporary
            eastl::any GetWaitableObject() const { return GetPrivateImpl()->GetWaitableObject(); }

        private:
            SwapChain(const SwapChainDesc& desc);

            // This method isn't thread safe. So it's should be called from device context.
            void Resize(uint32_t width, uint32_t height);

            inline void UpdateBackBuffer(Texture& resource, RenderTargetView& rtv) const { return GetPrivateImpl()->UpdateBackBuffer(resource, rtv); }

        private:
            SwapChainDesc desc_;

            friend class Render::DeviceContext;
            friend class Render::SwapChain;
        };
    }
}