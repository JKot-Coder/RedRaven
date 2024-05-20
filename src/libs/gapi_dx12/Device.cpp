#include "Device.hpp"

#include "gapi_dx12/DeviceImpl.hpp"

#include "gapi/Device.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            bool InitDevice(Device& device)
            {
                auto deviceImpl = std::make_unique<DeviceImpl>();

                if (!deviceImpl->Init(device.GetDescription()))
                    return false;

                device.SetPrivateImpl(deviceImpl.release());
                return true;
            }
        }
    }
}