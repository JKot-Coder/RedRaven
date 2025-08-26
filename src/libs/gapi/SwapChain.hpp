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
        public:
            SwapChainDesc() = default;
            SwapChainDesc(eastl::any windowNativeHandle, uint32_t width, uint32_t height, uint32_t bufferCount, GpuResourceFormat gpuResourceFormat, bool isStereo = false)
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
            GpuResourceFormat depthStencilFormat;

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
            virtual void InitDepthBufferTexture(Texture& resource) const = 0;
            virtual void Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFER_COUNT>& backBuffers) = 0;
        };

        class SwapChain final : public Resource<ISwapChain, false>
        {
        public:
            using UniquePtr = eastl::unique_ptr<SwapChain>;

            ~SwapChain();

            eastl::shared_ptr<Texture> GetBackBufferTexture(uint32_t index);
            eastl::shared_ptr<Texture> GetDepthBufferTexture();
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
            inline void InitDepthBufferTexture(Texture& resource) const { return GetPrivateImpl()->InitDepthBufferTexture(resource); }

        private:
            SwapChainDesc desc_;
            eastl::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT> backBuffers_;
            eastl::shared_ptr<Texture> depthBuffer_;

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };
    }
}