#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/Fence.hpp"

#include "gapi/GAPIStatus.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Resource;

        enum class CommandQueueType
        {
            GRAPHICS,
            COMPUTE,
            COPY,
            COUNT
        };

        enum class ResourceFormat
        {
            Unknown
        };

        struct PresentOptions
        {
            AlignedBox2i rect;

            Common::NativeWindowHandle windowHandle;

            ResourceFormat resourceFormat;
            uint32_t bufferCount;
            bool isStereo;
        };

        class SingleThreadDeviceInterface
        {
        public:
            virtual GAPIStatus Init() = 0;

            virtual GAPIStatus Reset(const PresentOptions& presentOptions) = 0;

            virtual GAPIStatus Present() = 0;
        };

        class MultiThreadDeviceInterface
        {
        public:
            virtual uint64_t GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const = 0;

            virtual GAPIStatus InitResource(Resource& resource) = 0;
        };

        class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
        {
        public:
            virtual ~Device() = default;
        };

    }
}
