#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Object.hpp"
#include "gapi/Result.hpp"

#include "common/NativeWindowHandle.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        struct SwapChainDescription
        {
            NativeWindowHandle windowHandle;

            uint32_t width;
            uint32_t height;
            uint32_t bufferCount;

            ResourceFormat resourceFormat;
            bool isStereo;

        public:
            SwapChainDescription() {};
            SwapChainDescription(NativeWindowHandle windowHandle, uint32_t width, uint32_t height, uint32_t bufferCount, ResourceFormat resourceFormat, bool isStereo = false)
                : windowHandle(windowHandle),
                  width(width),
                  height(height),
                  bufferCount(bufferCount),
                  resourceFormat(resourceFormat),
                  isStereo(isStereo)
            {
            }
        };

        class SwapChainInterface
        {
        public:
            virtual Result InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource) = 0;
        };

        class SwapChain final : public InterfaceWrapObject<SwapChainInterface>
        {
        public:
            using SharedPtr = std::shared_ptr<SwapChain>;
            using SharedConstPtr = std::shared_ptr<const SwapChain>;

            inline Result InitBackBufferTexture(uint32_t backBufferIndex, const std::shared_ptr<Texture>& resource) { return GetInterface()->InitBackBufferTexture(backBufferIndex, resource); }

            std::shared_ptr<Texture> GetTexture(uint32_t backBufferIndex);
            std::weak_ptr<CommandQueue> GetCommandQueue() { return commandQueue_; }

            const SwapChainDescription& GetDescription() const { return description_; }

        private:
            static SharedPtr Create(const std::shared_ptr<CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name)
            {
                return SharedPtr(new SwapChain(commandQueue, description, name));
            }

            SwapChain(const std::shared_ptr<CommandQueue>& commandQueue, const SwapChainDescription& description, const U8String& name);

        private:
            SwapChainDescription description_;
            std::array<std::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT> backBuffers_;
            std::weak_ptr<CommandQueue> commandQueue_;

            friend class Render::RenderContext;
        };
    }
}