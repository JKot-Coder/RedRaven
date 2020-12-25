#pragma once

#include "ForwardDeclarations.hpp"
#include "gapi/Object.hpp"

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

        class SwapChain final : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<SwapChain>;
            using SharedConstPtr = std::shared_ptr<const SwapChain>;

            SwapChain() = delete;

            const SwapChainDescription& GetDescription() const { return description_; }

        private:
            static SharedPtr Create(const SwapChainDescription& description, const U8String& name)
            {
                return SharedPtr(new SwapChain(description, name));
            }

            SwapChain(const SwapChainDescription& description, const U8String& name);

        private:
            SwapChainDescription description_;

            friend class RenderContext;
        };
    }
}