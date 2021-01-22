#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        class Device;

        namespace DX12
        {
            std::shared_ptr<Device> CreateDevice();
        }
    }
}