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

    private:
        bool inited = false;
        eastl::shared_ptr<GAPI::Device> device;
    };
}