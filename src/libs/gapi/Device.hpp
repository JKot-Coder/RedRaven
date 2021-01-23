#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Result.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class ISingleThreadDevice
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
            virtual Result MoveToNextFrame() = 0;
            virtual Result WaitForGpu() = 0;
        };

        class IMultiThreadDevice
        {
        public:
            virtual Result InitResource(const std::shared_ptr<Object>& resource) const = 0;
            virtual void ReleaseResource(Object& resource) const = 0;
        };

        class IDevice : public ISingleThreadDevice, public IMultiThreadDevice
        {
        public:
            virtual ~IDevice() = default;
        };

        class Device final : public Resource<IDevice>, public IDevice
        {
        public:
            using SharedPtr = std::shared_ptr<Device>;
            using SharedConstPtr = std::shared_ptr<const Device>;

        public:
            virtual ~Device() = default;

            Result Init(const Description& description) override { return GetPrivateImpl()->Init(description); };
            //   virtual Result Submit(const std::shared_ptr<CommandList>& CommandList) = 0;
            Result Present(const std::shared_ptr<SwapChain>& swapChain) override { return GetPrivateImpl()->Present(swapChain); }
            Result MoveToNextFrame() override { return GetPrivateImpl()->MoveToNextFrame(); }
            Result WaitForGpu() override { return GetPrivateImpl()->WaitForGpu(); }

            Result InitResource(const std::shared_ptr<Object>& resource) const override { return GetPrivateImpl()->InitResource(resource); }
            void ReleaseResource(Object& resource) const override { GetPrivateImpl()->ReleaseResource(resource); }

        public:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new Device(name));
            }

        private:
            Device(const U8String& name)
                : Resource(Object::Type::Device, name)
            {
            }
        };
    }
}