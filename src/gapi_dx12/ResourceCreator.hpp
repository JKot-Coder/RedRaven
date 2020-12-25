#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorHeapSet;

            struct ResourceCreatorContext
            {
                ComSharedPtr<ID3D12Device> device;
                ComSharedPtr<IDXGIFactory2> dxgiFactory;
                ComSharedPtr<ID3D12CommandQueue> graphicsCommandQueue;
                std::shared_ptr<DescriptorHeapSet> descriptorHeapSet;

                ResourceCreatorContext() = delete;

                ResourceCreatorContext(
                    const ComSharedPtr<ID3D12Device>& device,
                    const ComSharedPtr<IDXGIFactory2>& dxgiFactory,
                    const ComSharedPtr<ID3D12CommandQueue>& graphicsCommandQueue,
                    const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet)
                    : device(device),
                      dxgiFactory(dxgiFactory),
                      graphicsCommandQueue(graphicsCommandQueue),
                      descriptorHeapSet(descriptorHeapSet)
                {
                    ASSERT(device)
                    ASSERT(dxgiFactory)
                    ASSERT(graphicsCommandQueue)
                    ASSERT(descriptorHeapSet)
                }
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
                Result InitResource(const ResourceCreatorContext& context, const std::shared_ptr<Object>& resource);
            }
        }
    }
}