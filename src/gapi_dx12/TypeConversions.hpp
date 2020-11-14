#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            namespace TypeConversions
            {
                ::DXGI_FORMAT ResourceFormat(Resource::Format format);

                inline ::DXGI_FORMAT GetTypelessFormatFromDepthFormat(Resource::Format format)
                {
                    switch (format)
                    {
                    case Resource::Format::D16Unorm:
                        return DXGI_FORMAT_R16_TYPELESS;
                    case Resource::Format::D32FloatS8X24Uint:
                        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                    case Resource::Format::D24UnormS8Uint:
                        return DXGI_FORMAT_R24G8_TYPELESS;
                    case Resource::Format::D32Float:
                        return DXGI_FORMAT_R32_TYPELESS;
                    default:
                        ASSERT(!format.IsDepth());
                        return ResourceFormat(format);
                    }
                }

                D3D12_RESOURCE_FLAGS ResourceFlags(Resource::BindFlags flags);
            }
        }
    }
}