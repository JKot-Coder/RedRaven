#pragma once

#include "gapi/ForwardDeclarations.hpp"

namespace RR::RenderLoom
{
    class DeviceContext
    {
    public:
        DeviceContext() = default;
        ~DeviceContext();

        void Init(const GAPI::DeviceDescription& description);

        void Present(const eastl::shared_ptr<GAPI::SwapChain>& swapChain);
        eastl::shared_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description, const std::string& name = "") const;

    private:
        bool inited = false;
        eastl::shared_ptr<GAPI::Device> device;
    };
}