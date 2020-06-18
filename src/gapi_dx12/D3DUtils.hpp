#pragma once

#include "gapi_dx12/ComSharedPtr.hpp"

struct IDXGIAdapter1;
struct IDXGIFactory1;
namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                namespace D3DUtils
                {

                    void GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, ComSharedPtr<IDXGIAdapter1>& Adapter);

                }
            }
        }
    }
}