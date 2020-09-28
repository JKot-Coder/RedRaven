#pragma once

#include "gapi_dx12/DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Object;

        namespace DX12
        {

            struct ResourceCreatorContext
            {
                ID3D12Device* device;
                DescriptorHeapSet* descriptorHeapSet;
            };

            namespace ResourceCreator
            {
                Result InitResource(ResourceCreatorContext& context, Object& resource);
            }
        }
    }
}