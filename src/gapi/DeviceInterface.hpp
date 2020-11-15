#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/CommandContext.hpp"
#include "gapi/Fence.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Result.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Object;

        enum class CommandQueueType
        {
            GRAPHICS,
            COMPUTE,
            COPY,
            COUNT
        };

        struct PresentOptions
        {
            AlignedBox2i rect;

            Common::NativeWindowHandle windowHandle;

            Resource::Format resourceFormat;
            uint32_t bufferCount;
            bool isStereo;
        };

        class SingleThreadDeviceInterface
        {
        public:
            virtual Result Init() = 0;

            virtual Result Reset(const PresentOptions& presentOptions) = 0;

            virtual Result Submit(const CommandContext::SharedPtr& commandContext) = 0;

            virtual Result Present() = 0;
        };

        class MultiThreadDeviceInterface
        {
        public:
            virtual uint64_t GetGpuFenceValue(const Fence::SharedPtr& fence) const = 0;

            virtual Result InitResource(const Object::SharedPtr& resource) = 0;
        };

        class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
        {
        public:
            virtual ~Device() = default;
        };

    }
}
