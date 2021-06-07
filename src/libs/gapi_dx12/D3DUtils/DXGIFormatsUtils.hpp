#pragma once

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace D3DUtils
            {
                DXGI_FORMAT GetDxgiResourceFormat(GpuResourceFormat format);
                DXGI_FORMAT GetDxgiTypelessFormat(GpuResourceFormat format);
                DXGI_FORMAT SRGBToLinear(DXGI_FORMAT format);
            }
        }
    }
}