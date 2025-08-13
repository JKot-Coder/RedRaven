#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Resource.hpp"

// TODO temporary
#include <any>

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
            eastl::shared_ptr<Platform::Window> window;

            uint32_t width;
            uint32_t height;
            uint32_t bufferCount;

            GpuResourceFormat gpuResourceFormat;
            bool isStereo;

        public:
            SwapChainDescription() = default;
            SwapChainDescription(const eastl::shared_ptr<Platform::Window>& window, uint32_t width, uint32_t height, uint32_t bufferCount, GpuResourceFormat gpuResourceFormat, bool isStereo = false)
                : window(window),
                  width(width),
                  height(height),
                  bufferCount(bufferCount),
                  gpuResourceFormat(gpuResourceFormat),
                  isStereo(isStereo)
            {
            }
        };

        class ISwapChain
        {
        public:
            virtual ~ISwapChain() {};

            virtual uint32_t GetCurrentBackBufferIndex() const = 0;
            virtual std::any GetWaitableObject() const = 0;

            virtual void InitBackBufferTexture(uint32_t backBufferIndex, const eastl::shared_ptr<Texture>& resource) = 0;
            virtual void Reset(const SwapChainDescription& description, const std::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers) = 0;
        };

        class SwapChain final : public Resource<ISwapChain>
        {
        public:
            using SharedPtr = eastl::shared_ptr<SwapChain>;
            using SharedConstPtr = eastl::shared_ptr<const SwapChain>;

            eastl::shared_ptr<Texture> GetBackBufferTexture(uint32_t index);

            const SwapChainDescription& GetDescription() const { return description_; }
            uint32_t GetCurrentBackBufferIndex() const { return GetPrivateImpl()->GetCurrentBackBufferIndex(); }

            // TODO temporary
            std::any GetWaitableObject() const { return GetPrivateImpl()->GetWaitableObject(); }

        private:
            static SharedPtr Create(const SwapChainDescription& description, const std::string& name)
            {
                return SharedPtr(new SwapChain(description, name));
            }

            SwapChain(const SwapChainDescription& description, const std::string& name);

            void Reset(const SwapChainDescription& description);

            inline void InitBackBufferTexture(uint32_t backBufferIndex, const eastl::shared_ptr<Texture>& resource) { return GetPrivateImpl()->InitBackBufferTexture(backBufferIndex, resource); }

        private:
            SwapChainDescription description_;
            std::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT> backBuffers_;

            friend class Render::DeviceContext;
        };
    }
}