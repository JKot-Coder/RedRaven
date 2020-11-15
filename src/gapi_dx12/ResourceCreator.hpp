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
                ComSharedPtr<ID3D12Device> device;
                std::shared_ptr<DescriptorHeapSet> descriptorHeapSet;
            };

            static constexpr D3D12_HEAP_PROPERTIES DefaultHeapProps = {
                D3D12_HEAP_TYPE_DEFAULT,
                D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                D3D12_MEMORY_POOL_UNKNOWN,
                0,
                0
            };

            static constexpr D3D12_HEAP_PROPERTIES UploadHeapProps = {
                D3D12_HEAP_TYPE_UPLOAD,
                D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                D3D12_MEMORY_POOL_UNKNOWN,
                0,
                0,
            };

            static constexpr D3D12_HEAP_PROPERTIES ReadbackHeapProps = {
                D3D12_HEAP_TYPE_READBACK,
                D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                D3D12_MEMORY_POOL_UNKNOWN,
                0,
                0
            };

            namespace ResourceCreator
            {
                Result InitResource(const ResourceCreatorContext& context, const Object::SharedPtr& resource);
            }
        }
    }
}