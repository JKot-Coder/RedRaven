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

                GAPIStatus Init() override;
                GAPIStatus Reset(const PresentOptions& presentOptions) override;
                GAPIStatus Present() override;

                uint64_t GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const override;

                GAPIStatus InitResource(Resource& resource) override;

                void WaitForGpu();

            private:

                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }        
    }
}