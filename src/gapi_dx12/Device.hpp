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
                ~Device();

                // Todo separate freeThread / Multithread
                Result Init(const Description& description) override;
                Result Present(const std::shared_ptr<SwapChain>& swapChain) override;

                //  Result Submit(const std::shared_ptr<CommandList>& CommandList) override;

                Result InitResource(const std::shared_ptr<Object>& resource) const override;

                Result WaitForGpu() override;

            private:
                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }
    }
}