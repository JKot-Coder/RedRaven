#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Resource.hpp"

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
            std::shared_ptr<Platform::Window> window;

            uint32_t width;
            uint32_t height;
            uint32_t bufferCount;

            GpuResourceFormat gpuResourceFormat;
            bool isStereo;

        public:
            SwapChainDescription() = default;
            SwapChainDescription(const std::shared_ptr<Platform::Window>& window, uint32_t width, uint32_t height, uint32_t bufferCount, GpuResourceFormat gpuResourceFormat, bool isStereo = false)
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

            virtual void InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource) = 0;

            virtual void Reset(const SwapChainDescription& description, const std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers) = 0;
        };

        class SwapChain final : public Resource<ISwapChain>
        {
        public:
            using SharedPtr = std::shared_ptr<SwapChain>;
            using SharedConstPtr = std::shared_ptr<const SwapChain>;

            std::shared_ptr<Texture> GetTexture(uint32_t backBufferIndex);

            const SwapChainDescription& GetDescription() const { return description_; }

        private:
            static SharedPtr Create(const SwapChainDescription& description, const U8String& name)
            {
                return SharedPtr(new SwapChain(description, name));
            }

            SwapChain(const SwapChainDescription& description, const U8String& name);

            void Reset(const SwapChainDescription& description);

            inline void InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource) { return GetPrivateImpl()->InitBackBufferTexture(backBufferIndex, resource); }

        private:
            SwapChainDescription description_;
            std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT> backBuffers_;

            friend class Render::DeviceContext;
        };
    }
}