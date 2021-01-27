#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
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
                void InitSwapChain(SwapChain& resource);
                void InitFence(Fence& resource);
                void InitCommandQueue(CommandQueue& resource);
                void InitCommandList(CommandList& resource);
                void InitGpuResourceView(GpuResourceView& view);

                void ReleaseResource(Object& resource);
            }
        }
    }
}