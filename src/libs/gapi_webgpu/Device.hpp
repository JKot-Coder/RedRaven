#pragma once

namespace RR::GAPI
{
    class Device;

    namespace WebGPU
    {
        bool InitDevice(Device& device);
    }
}