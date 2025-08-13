#include "DeviceContext.hpp"

#include "gapi/SwapChain.hpp"
#include "gapi/Device.hpp"

#include "gapi_diligent/Device.hpp"

namespace RR::RenderLoom
{
    DeviceContext::~DeviceContext() { }

    void DeviceContext::Init(const GAPI::DeviceDescription& description)
    {
        ASSERT(!inited);
        // TODO: device should be belong to submission thread
        device = GAPI::Device::Create(description, "Primary");

        // Todo: select device based on description
        // Todo: do it in submission thread
        GAPI::Diligent::InitDevice(device);
        inited = true;
    }

}
