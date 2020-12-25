#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace TypeConversions
            {
                ::DXGI_FORMAT GetResourceFormat(ResourceFormat format);

                inline ::DXGI_FORMAT GetTypelessFormatFromDepthFormat(ResourceFormat format)
                {
                    switch (format)
                    {
                    case ResourceFormat::D16Unorm:
                        return DXGI_FORMAT_R16_TYPELESS;
                    case ResourceFormat::D32FloatS8X24Uint:
                        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                    case ResourceFormat::D24UnormS8Uint:
                        return DXGI_FORMAT_R24G8_TYPELESS;
                    case ResourceFormat::D32Float:
                        return DXGI_FORMAT_R32_TYPELESS;
                    default:
                        ASSERT(!ResourceFormatInfo::IsDepth(format));
                        return GetResourceFormat(format);
                    }
                }

                D3D12_RESOURCE_FLAGS GetResourceFlags(Resource::BindFlags flags);
            }
        }
    }
}