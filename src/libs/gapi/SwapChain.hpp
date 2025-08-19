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
        struct SwapChainDescription
        {
        public:
            SwapChainDescription() = default;
            SwapChainDescription(eastl::any windowNativeHandle, uint32_t width, uint32_t height, uint32_t bufferCount, GpuResourceFormat gpuResourceFormat, bool isStereo = false)
                : width(width),
                  height(height),
                  bufferCount(bufferCount),
                  gpuResourceFormat(gpuResourceFormat),
                  isStereo(isStereo),
                  windowNativeHandle(eastl::move(windowNativeHandle))
            {
            }

        public:
            uint32_t width;
            uint32_t height;
            uint32_t bufferCount;

            GpuResourceFormat gpuResourceFormat;
            bool isStereo;

            eastl::any windowNativeHandle;
        };

        class ISwapChain
        {
        public:
            virtual ~ISwapChain() {};

            virtual uint32_t GetCurrentBackBufferIndex() const = 0;
            virtual eastl::any GetWaitableObject() const = 0;

            virtual void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const = 0;
            virtual void Reset(const SwapChainDescription& description, const RR::GAPI::Texture** backBuffers) = 0;
        };

        class SwapChain final : public Resource<ISwapChain, false>
        {
        public:
            using UniquePtr = eastl::unique_ptr<SwapChain>;

            ~SwapChain();

            Texture* GetBackBufferTexture(uint32_t index);
            Texture* GetCurrentBackBufferTexture() { return GetBackBufferTexture(GetCurrentBackBufferIndex()); }

            const SwapChainDescription& GetDescription() const { return description_; }
            uint32_t GetCurrentBackBufferIndex() const { return GetPrivateImpl()->GetCurrentBackBufferIndex(); }

            // TODO temporary
            eastl::any GetWaitableObject() const { return GetPrivateImpl()->GetWaitableObject(); }

        private:
            static UniquePtr Create(const SwapChainDescription& description)
            {
                return UniquePtr(new SwapChain(description));
            }

            SwapChain(const SwapChainDescription& description);

            void Reset(const SwapChainDescription& description);

            inline void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const { return GetPrivateImpl()->InitBackBufferTexture(backBufferIndex, resource); }

        private:
            SwapChainDescription description_;
            eastl::array<eastl::unique_ptr<Texture>, MAX_BACK_BUFFER_COUNT> backBuffers_;

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };
    }
}