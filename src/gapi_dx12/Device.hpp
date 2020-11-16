#include "gapi/DeviceInterface.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class Device : public Render::Device
            {
            public:
                Device();
                ~Device() override;

                // Todo separate freeThread / Multithread
                Result Init() override;
                Result Reset(const PresentOptions& presentOptions) override;
                Result ResetSwapchain(const std::shared_ptr<SwapChain>& oldSwapChain, const std::shared_ptr<SwapChain>& newSwapChain) override;
                Result Present() override;

                Result Submit(const std::shared_ptr<CommandContext>& commandContext) override;

                uint64_t GetGpuFenceValue(const std::shared_ptr<Fence>& fence) const override;

                Result InitResource(const std::shared_ptr<Object>& resource) override;

                void WaitForGpu();

            private:
                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }
    }
}