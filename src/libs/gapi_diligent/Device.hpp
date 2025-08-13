#pragma once

namespace RR::GAPI
{
    class Device;

    namespace Diligent
    {
        bool InitDevice(eastl::shared_ptr<Device>& device);
    }
}