#include "Device.hpp"

#include "gapi_diligent/DeviceImpl.hpp"

#include "gapi/Device.hpp"

namespace RR::GAPI::Diligent
{
    bool InitDevice(GAPI::Device::SharedPtr& device)
    {
        auto deviceImpl = std::make_unique<DeviceImpl>();

        if (!deviceImpl->Init(device->GetDescription()))
            return false;

        device->SetPrivateImpl(deviceImpl.release());
        return true;
    }
}