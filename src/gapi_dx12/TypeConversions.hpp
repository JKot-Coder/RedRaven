#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace TypeConversions
            {
                //TODO Resource Format conrsions 
                ::DXGI_FORMAT GetGpuResourceFormat(GpuResourceFormat format);

                inline ::DXGI_FORMAT GetTypelessFormatFromDepthFormat(GpuResourceFormat format)
                {
                    switch (format)
                    {
                    case GpuResourceFormat::D16Unorm:
                        return DXGI_FORMAT_R16_TYPELESS;
                    case GpuResourceFormat::D32FloatS8X24Uint:
                        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                    case GpuResourceFormat::D24UnormS8Uint:
                        return DXGI_FORMAT_R24G8_TYPELESS;
                    case GpuResourceFormat::D32Float:
                        return DXGI_FORMAT_R32_TYPELESS;
                    default:
                        ASSERT(!GpuResourceFormatInfo::IsDepth(format));
                        return GetGpuResourceFormat(format);
                    }
                }

                D3D12_RESOURCE_FLAGS GetResourceFlags(GpuResourceBindFlags flags);
            }
        }
    }
}