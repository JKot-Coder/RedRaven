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

                GAPIResult Init() override;
                GAPIResult Reset(const PresentOptions& presentOptions) override;
                GAPIResult Present() override;

                uint64_t GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const override;

                GAPIResult InitResource(Resource& resource) override;

                void WaitForGpu();

            private:

                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }        
    }
}