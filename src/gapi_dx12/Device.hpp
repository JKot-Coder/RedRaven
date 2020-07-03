#include "gapi/Device.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                class Device : public Render::Device::Device
                {
                public:
                    Device();
                    ~Device() override;

                    GAPIStatus Init() override;
                    GAPIStatus Reset(const PresentOptions& presentOptions) override;
                    GAPIStatus Present() override;

                    GAPIStatus CompileCommandList(CommandList& commandList) const override;
                    GAPIStatus SubmitCommandList(CommandList& commandList) const override;

                    uint64_t GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const override;

                    GAPIStatus InitResource(CommandList& commandList) const override;

                    void WaitForGpu();

                private:

                    std::unique_ptr<class DeviceImplementation> _impl;
                };

            }
        }
    }
}