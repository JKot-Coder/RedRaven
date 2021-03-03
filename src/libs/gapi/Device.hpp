#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

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
            //   virtual void Submit(const std::shared_ptr<CommandList>& CommandList) = 0;
            virtual void Present(const std::shared_ptr<SwapChain>& swapChain) = 0;
            virtual void MoveToNextFrame(uint64_t frameIndex) = 0;
        };

        class IMultiThreadDevice
        {
        public:
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            virtual std::shared_ptr<IntermediateMemory> const AllocateIntermediateTextureData(
                const TextureDescription& desc,
                MemoryAllocationType memoryType,
                uint32_t firstSubresourceIndex = 0,
                uint32_t numSubresources = MaxPossible) const = 0;

            virtual void InitSwapChain(SwapChain& resource) const = 0;
            virtual void InitFence(Fence& resource) const = 0;
            virtual void InitCommandQueue(CommandQueue& resource) const = 0;
            virtual void InitCommandList(CommandList& resource) const = 0;
            virtual void InitTexture(Texture& resource) const = 0;
            virtual void InitBuffer(Buffer& resource) const = 0;
            virtual void InitGpuResourceView(GpuResourceView& view) const = 0;

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

            const Description& GetDescription() const { return description_; }

            //   virtual void Submit(const std::shared_ptr<CommandList>& CommandList) = 0;
            void Present(const std::shared_ptr<SwapChain>& swapChain) override { GetPrivateImpl()->Present(swapChain); }
            void MoveToNextFrame(uint64_t frameIndex) override { GetPrivateImpl()->MoveToNextFrame(frameIndex); }

            std::shared_ptr<IntermediateMemory> const AllocateIntermediateTextureData(
                const TextureDescription& desc,
                MemoryAllocationType memoryType,
                uint32_t firstSubresourceIndex = 0,
                uint32_t numSubresources = MaxPossible) const override
            {
                return GetPrivateImpl()->AllocateIntermediateTextureData(desc, memoryType, firstSubresourceIndex, numSubresources);
            };

            void InitSwapChain(SwapChain& resource) const override { GetPrivateImpl()->InitSwapChain(resource); };
            void InitFence(Fence& resource) const override { GetPrivateImpl()->InitFence(resource); };
            void InitCommandQueue(CommandQueue& resource) const override { GetPrivateImpl()->InitCommandQueue(resource); };
            void InitCommandList(CommandList& resource) const override { GetPrivateImpl()->InitCommandList(resource); };
            void InitTexture(Texture& resource) const override { GetPrivateImpl()->InitTexture(resource); };
            void InitBuffer(Buffer& resource) const override { GetPrivateImpl()->InitBuffer(resource); };
            void InitGpuResourceView(GpuResourceView& view) const override { GetPrivateImpl()->InitGpuResourceView(view); };

            void ReleaseResource(Object& resource) const override { GetPrivateImpl()->ReleaseResource(resource); }

        private:
            static SharedPtr Create(const Description& description, const U8String& name)
            {
                return std::shared_ptr<Device>(new Device(description, name));
            }

            Device(const Description& description, const U8String& name)
                : Resource(Object::Type::Device, name),
                  description_(description)
            {
            }

            friend class Render::RenderContext;

        private:
            Description description_;
        };
    }
}