#pragma once

#include "gapi_dx12/DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Resource;

        namespace DX12
        {

            struct ResourceCreatorContext
            {
                ID3D12Device* device;
                DescriptorHeapSet* descriptorHeapSet;
            };

            namespace ResourceCreator
            {
                GAPIResult InitResource(ResourceCreatorContext& context, Resource& resource);
            }
        }
    }
}