#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorHeapSet;
            class CommandQueueImpl;
            class ResourceReleaseContext;

            struct ResourceCreateContext
            {
                ComSharedPtr<ID3D12Device> device;
                ComSharedPtr<IDXGIFactory2> dxgiFactory;
                std::shared_ptr<CommandQueueImpl> graphicsCommandQueue;
                std::shared_ptr<DescriptorHeapSet> descriptorHeapSet;

                ResourceCreateContext() = delete;

                ResourceCreateContext(
                    const ComSharedPtr<ID3D12Device>& device,
                    const ComSharedPtr<IDXGIFactory2>& dxgiFactory,
                    const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                    const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet)
                    : device(device),
                      dxgiFactory(dxgiFactory),
                      graphicsCommandQueue(graphicsCommandQueue),
                      descriptorHeapSet(descriptorHeapSet)
                {
                    ASSERT(device)
                    ASSERT(dxgiFactory)
                    ASSERT(descriptorHeapSet)
                    ASSERT(graphicsCommandQueue)
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
                Result InitResource(const ResourceCreateContext& context, const std::shared_ptr<Object>& resource);
                void ReleaseResource(ResourceReleaseContext& resourceReleaseContext, Object& resource);
            }
        }
    }
}