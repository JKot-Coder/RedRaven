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

                Result Init() override;
                Result Reset(const PresentOptions& presentOptions) override;
                Result Present() override;

                uint64_t GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const override;

                Result InitResource(Object::ConstSharedPtrRef resource) override;

                void WaitForGpu();

            private:

                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }        
    }
}