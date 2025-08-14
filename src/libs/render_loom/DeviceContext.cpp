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
        if(!GAPI::Diligent::InitDevice(device))
        {
            Log::Format::Error("Failed to initialize device");
            return;
        }

        inited = true;
    }

    void DeviceContext::Present(const GAPI::SwapChain::SharedPtr& swapChain)
    {
        device->Present(swapChain);
    }

    GAPI::SwapChain::SharedPtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDescription& description, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::SwapChain::Create(description, name);
        device->InitSwapChain(*resource.get());

        return resource;
    }
}
