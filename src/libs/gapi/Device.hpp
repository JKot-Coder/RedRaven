#pragma once

#include "math/Math.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/Resource.hpp"

#include <any>

namespace RR
{
    namespace GAPI
    {
        struct DeviceDescription final
        {
        public:
            enum class DebugMode : uint32_t
            {
                Retail,
                Instrumented,
                Debug
            };

        public:
            DeviceDescription() = default;
            DeviceDescription(DebugMode debugMode) : debugMode(debugMode) { }

        public:
            DebugMode debugMode = DebugMode::Retail;
            uint32_t maxFramesInFlight = 2;
        };

        class ISingleThreadDevice
        {
        public:

        public:
            //   virtual void Submit(const eastl::shared_ptr<CommandList>& CommandList) = 0;
            virtual void Present(SwapChain* swapChain) = 0;
            virtual void MoveToNextFrame(uint64_t frameIndex) = 0;
        };

        class IMultiThreadDevice
        {
        public:
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            virtual GAPI::GpuResourceFootprint GetResourceFootprint(const GpuResourceDescription& description) const = 0;

            virtual void Compile(CommandContext* commandContext) = 0;

            virtual void InitBuffer(Buffer& resource) const = 0;
            virtual void InitCommandList(CommandList& resource) const = 0;
            virtual void InitCommandContext(CommandContext& resource) const = 0;
            virtual void InitCommandQueue(CommandQueue& resource) const = 0;
            virtual void InitFence(Fence& resource) const = 0;
            virtual void InitFramebuffer(Framebuffer& resource) const = 0;
            virtual void InitGpuResourceView(GpuResourceView& view) const = 0;
            virtual void InitSwapChain(SwapChain& resource) const = 0;
            virtual void InitTexture(Texture& resource) const = 0;

            virtual std::any GetRawDevice() const = 0;
        };

        class IDevice : public ISingleThreadDevice, public IMultiThreadDevice
        {
        public:
            virtual ~IDevice() = default;
        };

        class Device final : public Resource<IDevice>, public IDevice
        {
        public:
            using SharedPtr = eastl::shared_ptr<Device>;
            using SharedConstPtr = eastl::shared_ptr<const Device>;

        public:
            virtual ~Device() = default;

            const DeviceDescription& GetDescription() const { return description_; }

            //   virtual void Submit(const eastl::shared_ptr<CommandList>& CommandList) = 0;
            void Present(SwapChain* swapChain) override { GetPrivateImpl()->Present(swapChain); }
            void MoveToNextFrame(uint64_t frameIndex) override { GetPrivateImpl()->MoveToNextFrame(frameIndex); }
            void Compile(CommandContext* commandContext) override { GetPrivateImpl()->Compile(commandContext); }

            GAPI::GpuResourceFootprint GetResourceFootprint(const GpuResourceDescription& description) const
                override { return GetPrivateImpl()->GetResourceFootprint(description); };

            // Todo init resource?
            void InitBuffer(Buffer& resource) const override { GetPrivateImpl()->InitBuffer(resource); };
            void InitCommandList(CommandList& resource) const override { GetPrivateImpl()->InitCommandList(resource); };
            void InitCommandContext(CommandContext& resource) const override { GetPrivateImpl()->InitCommandContext(resource); };
            void InitCommandQueue(CommandQueue& resource) const override { GetPrivateImpl()->InitCommandQueue(resource); };
            void InitFence(Fence& resource) const override { GetPrivateImpl()->InitFence(resource); };
            void InitFramebuffer(Framebuffer& resource) const override { GetPrivateImpl()->InitFramebuffer(resource); };
            void InitGpuResourceView(GpuResourceView& view) const override { GetPrivateImpl()->InitGpuResourceView(view); };
            void InitSwapChain(SwapChain& resource) const override { GetPrivateImpl()->InitSwapChain(resource); };
            void InitTexture(Texture& resource) const override { GetPrivateImpl()->InitTexture(resource); };

            std::any GetRawDevice() const override { return GetPrivateImpl()->GetRawDevice(); }

        private:
            static SharedPtr Create(const DeviceDescription& description, const std::string& name)
            {
                return eastl::shared_ptr<Device>(new Device(description, name));
            }

            Device(const DeviceDescription& description, const std::string& name)
                : Resource(Type::Device, name),
                  description_(description)
            {
            }

            friend class Render::DeviceContext;
            friend class RenderLoom::DeviceContext;

        private:
            DeviceDescription description_;
        };
    }
}