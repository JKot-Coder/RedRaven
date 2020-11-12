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
                D3D12_RESOURCE_FLAGS ResourceFlags(Resource::BindFlags flags);
            }
        }
    }
}