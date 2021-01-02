#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class SingleThreadDeviceInterface
        {
        public:
            enum class DebugMode : uint32_t
            {
                Retail,
                Instrumented,
                Debug
            };

            struct Description final
            {
            public:
                Description() = default;	

                Description(uint32_t gpuFramesBuffered, DebugMode debugMode)
                    : gpuFramesBuffered(gpuFramesBuffered),
                      debugMode(debugMode)
                {
                }

            public:
                uint32_t gpuFramesBuffered = 0;
                DebugMode debugMode = DebugMode::Retail;
            };

        public:
            virtual Result Init(const Description& description) = 0;

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