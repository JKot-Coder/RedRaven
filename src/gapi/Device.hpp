#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/GAPIStatus.hpp"

namespace OpenDemo
{
    namespace Render
    {
        enum class ResourceFormat
        {
            Unknown
        };

        namespace Device
        {
            struct PresentOptions
            {
                Common::RectU rect;

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
            };

            class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
            {
            public:
                virtual ~Device() = default;
            };

        }
    }
}
