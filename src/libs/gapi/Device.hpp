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
        struct DeviceDesc final
        {
        public:
            enum class DebugMode : uint32_t
            {
                Retail,
                Instrumented,
                Debug
            };

        public:
            DeviceDesc() = default;
            DeviceDesc(DebugMode debugMode) : debugMode(debugMode) { }

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

            virtual GAPI::GpuResourceFootprint GetResourceFootprint(const GpuResourceDesc& desc) const = 0;

            virtual void Compile(CommandList2& commandList) = 0;

            virtual void InitBuffer(Buffer& resource) const = 0;
            virtual void InitCommandList(CommandList& resource) const = 0;
            virtual void InitCommandList2(CommandList2& resource) const = 0;
            virtual void InitCommandQueue(CommandQueue& resource) const = 0;
            virtual void InitFence(Fence& resource) const = 0;
            virtual void InitGpuResourceView(GpuResourceView& view) const = 0;
            virtual void InitSwapChain(SwapChain& resource) const = 0;
            virtual void InitTexture(Texture& resource) const = 0;
            virtual void InitShader(Shader& resource) const = 0;
            virtual void InitPipelineState(PipelineState& resource) const = 0;

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

            const DeviceDesc& GetDesc() const { return desc_; }

            //   virtual void Submit(const eastl::shared_ptr<CommandList>& CommandList) = 0;
            void Present(SwapChain* swapChain) override { GetPrivateImpl()->Present(swapChain); }
            void MoveToNextFrame(uint64_t frameIndex) override { GetPrivateImpl()->MoveToNextFrame(frameIndex); }
            void Compile(CommandList2& commandList) override { GetPrivateImpl()->Compile(commandList); }

            GAPI::GpuResourceFootprint GetResourceFootprint(const GpuResourceDesc& desc) const
                override { return GetPrivateImpl()->GetResourceFootprint(desc); };

            // Todo init resource?
            void InitBuffer(Buffer& resource) const override { GetPrivateImpl()->InitBuffer(resource); };
            void InitCommandList(CommandList& resource) const override { GetPrivateImpl()->InitCommandList(resource); };
            void InitCommandList2(CommandList2& resource) const override { GetPrivateImpl()->InitCommandList2(resource); };
            void InitCommandQueue(CommandQueue& resource) const override { GetPrivateImpl()->InitCommandQueue(resource); };
            void InitFence(Fence& resource) const override { GetPrivateImpl()->InitFence(resource); };
            void InitGpuResourceView(GpuResourceView& view) const override { GetPrivateImpl()->InitGpuResourceView(view); };
            void InitSwapChain(SwapChain& resource) const override { GetPrivateImpl()->InitSwapChain(resource); };
            void InitTexture(Texture& resource) const override { GetPrivateImpl()->InitTexture(resource); };
            void InitShader(Shader& resource) const override { GetPrivateImpl()->InitShader(resource); };
            void InitPipelineState(PipelineState& resource) const override { GetPrivateImpl()->InitPipelineState(resource); };

            std::any GetRawDevice() const override { return GetPrivateImpl()->GetRawDevice(); }

        private:
            static SharedPtr Create(const DeviceDesc& desc, const std::string& name)
            {
                return eastl::shared_ptr<Device>(new Device(desc, name));
            }

            Device(const DeviceDesc& desc, const std::string& name)
                : Resource(Type::Device, name),
                  desc_(desc)
            {
            }

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;

        private:
            DeviceDesc desc_;
        };
    }
}