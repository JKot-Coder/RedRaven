#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Resource.hpp"

// TODO temporary
#include <eastl/any.h>

namespace RR
{
    namespace Platform
    {
        class Window;
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

            virtual uint32_t GetCurrentBackBufferIndex() const = 0;
            virtual eastl::any GetWaitableObject() const = 0;

            virtual void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const = 0;
            virtual void Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFERS_COUNT>& backBuffers) = 0;
        };

        class SwapChain final : public Resource<ISwapChain, false>
        {
        public:
            using UniquePtr = eastl::unique_ptr<SwapChain>;

            ~SwapChain();

            eastl::shared_ptr<Texture> GetBackBufferTexture(uint32_t index);
            eastl::shared_ptr<Texture> GetCurrentBackBufferTexture() { return GetBackBufferTexture(GetCurrentBackBufferIndex()); }

            const SwapChainDesc& GetDesc() const { return desc_; }
            uint32_t GetCurrentBackBufferIndex() const { return GetPrivateImpl()->GetCurrentBackBufferIndex(); }

            // TODO temporary
            eastl::any GetWaitableObject() const { return GetPrivateImpl()->GetWaitableObject(); }

        private:
            static UniquePtr Create(const SwapChainDesc& desc)
            {
                return UniquePtr(new SwapChain(desc));
            }

            SwapChain(const SwapChainDesc& desc);

            // This method isn't thread safe. So it's should be called from device context.
            void Resize(uint32_t width, uint32_t height);

            inline void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const { return GetPrivateImpl()->InitBackBufferTexture(backBufferIndex, resource); }

        private:
            SwapChainDesc desc_;
            eastl::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFERS_COUNT> backBuffers_;

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };
    }
}