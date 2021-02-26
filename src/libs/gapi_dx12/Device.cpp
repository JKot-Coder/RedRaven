#include "Device.hpp"

#include "gapi_dx12/DeviceImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            std::shared_ptr<Device> InitDevice()
            {
                auto& device = Device::Create("Primary");
                ASSERT(device);

                device->SetPrivateImpl(new DeviceImpl());
                return device;
            }
        }
    }
}