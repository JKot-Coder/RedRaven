#pragma once

namespace RR
{
    namespace GAPI::DX12
    {
        namespace D3DUtils
        {
            DXGI_SAMPLE_DESC GetSampleDesc(MultisampleType multisampleType);
            DXGI_FORMAT GetDxgiResourceFormat(GpuResourceFormat format);
            DXGI_FORMAT GetDxgiTypelessFormat(GpuResourceFormat format);
            DXGI_FORMAT SRGBToLinear(DXGI_FORMAT format);
        }
    }
}