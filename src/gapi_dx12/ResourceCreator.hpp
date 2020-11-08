#pragma once

#include "gapi/Object.hpp"
#include "gapi_dx12/DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace Render
    {

        namespace DX12
        {

            struct ResourceCreatorContext
            {
                ID3D12Device* device;
                DescriptorHeapSet* descriptorHeapSet;
            };

            namespace ResourceCreator
            {
                Result InitResource(ResourceCreatorContext& context, Object::ConstSharedPtrRef resource);
            }
        }
    }
}