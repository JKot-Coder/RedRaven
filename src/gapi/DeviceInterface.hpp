#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        struct PresentOptions
        {
            AlignedBox2i rect;

            NativeWindowHandle windowHandle;

            ResourceFormat resourceFormat;
            uint32_t bufferCount;
            bool isStereo;
        };

        class SingleThreadDeviceInterface
        {
        public:
            virtual Result Init() = 0;

            virtual Result Reset(const PresentOptions& presentOptions) = 0;

            virtual Result ResetSwapchain(const std::shared_ptr<SwapChain>& swapChain, const SwapChainDescription& description) = 0;

            virtual Result Submit(const std::shared_ptr<CommandList>& CommandList) = 0;

            virtual Result Present() = 0;
        };

        class MultiThreadDeviceInterface
        {
        public:
            virtual uint64_t GetGpuFenceValue(const std::shared_ptr<Fence>& fence) const = 0;

            virtual Result InitResource(const std::shared_ptr<Object>& resource) const = 0;
            virtual Result InitResource(const std::shared_ptr<Fence>& fence, uint64_t initialValue) const = 0;
        };

        class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
        {
        public:
            virtual ~Device() = default;
        };
    }
}
