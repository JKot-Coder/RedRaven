#include "Device.hpp"

#include "DeviceImpl.hpp"

#include "gapi/Device.hpp"

namespace RR::GAPI::WebGPU
{
    bool InitDevice(GAPI::Device& device)
    {
        auto deviceImpl = std::make_unique<DeviceImpl>();

        if (!deviceImpl->Init(device.GetDesc()))
            return false;

        device.SetPrivateImpl(deviceImpl.release());
        return true;
    }
}