#include "gapi/DeviceInterface.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class Device final : public GAPI::Device
            {
            public:
                Device();
                ~Device() override;

                // Todo separate freeThread / Multithread
                Result Init() override;
                Result Reset(const PresentOptions& presentOptions) override;
                Result ResetSwapchain(const std::shared_ptr<SwapChain>& swapChain, const SwapChainDescription& description) override;
                Result Present() override;

                //  Result Submit(const std::shared_ptr<CommandList>& CommandList) override;

                Result InitResource(const std::shared_ptr<Object>& resource) const override;

                void WaitForGpu();

            private:
                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }
    }
}