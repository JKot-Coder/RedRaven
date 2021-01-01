#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class SingleThreadDeviceInterface
        {
        public:
            virtual Result Init() = 0;

            //   virtual Result Submit(const std::shared_ptr<CommandList>& CommandList) = 0;

            virtual Result Present(const std::shared_ptr<SwapChain>& swapChain) = 0;

            virtual Result WaitForGpu() = 0;
        };

        class MultiThreadDeviceInterface
        {
        public:
            virtual Result InitResource(const std::shared_ptr<Object>& resource) const = 0;
        };

        class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
        {
        public:
            virtual ~Device() = default;
        };
    }
}
