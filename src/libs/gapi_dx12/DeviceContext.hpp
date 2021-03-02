#pragma once

namespace D3D12MA
{
    class Allocator;
}

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceReleaseContext;
            class DescriptorHeapSet;
            class CommandQueueImpl;
            class GpuMemoryHeap;

            class DeviceContext
            {
            public:
                static void Init(const ComSharedPtr<ID3D12Device>& device,
                                 const ComSharedPtr<IDXGIFactory2>& dxgiFactory);

                static void Init(D3D12MA::Allocator* allocator,
                                 const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                                 const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet,
                                 const std::shared_ptr<ResourceReleaseContext>& resourceReleaseContext,
                                 const std::shared_ptr<GpuMemoryHeap>& uploadHeap,
                                 const std::shared_ptr<GpuMemoryHeap>& readbackHeap);

                static void Terminate();

                static D3D12MA::Allocator* GetAllocator()
                {
                    ASSERT(allocator_);
                    return allocator_;
                }

                static ComSharedPtr<ID3D12Device> GetDevice()
                {
                    ASSERT(device_);
                    return device_;
                }

                static ComSharedPtr<IDXGIFactory2> GetDxgiFactory()
                {
                    ASSERT(dxgiFactory_);
                    return dxgiFactory_;
                }

                static std::shared_ptr<CommandQueueImpl> GetGraphicsCommandQueue()
                {
                    ASSERT(graphicsCommandQueue_);
                    return graphicsCommandQueue_;
                }

                static std::shared_ptr<DescriptorHeapSet> GetDesciptorHeapSet()
                {
                    ASSERT(descriptorHeapSet_);
                    return descriptorHeapSet_;
                }

                static std::shared_ptr<ResourceReleaseContext> GetResourceReleaseContext()
                {
                    ASSERT(resourceReleaseContext_);
                    return resourceReleaseContext_;
                }

                static std::shared_ptr<GpuMemoryHeap> GetUploadHeap()
                {
                    ASSERT(uploadHeap_);
                    return uploadHeap_;
                }

                static std::shared_ptr<GpuMemoryHeap> GetReadbackHeap()
                {
                    ASSERT(readbackHeap_);
                    return readbackHeap_;
                }

            private:
                static D3D12MA::Allocator* allocator_;
                static ComSharedPtr<ID3D12Device> device_;
                static ComSharedPtr<IDXGIFactory2> dxgiFactory_;
                static std::shared_ptr<CommandQueueImpl> graphicsCommandQueue_;
                static std::shared_ptr<DescriptorHeapSet> descriptorHeapSet_;
                static std::shared_ptr<ResourceReleaseContext> resourceReleaseContext_;
                static std::shared_ptr<GpuMemoryHeap> uploadHeap_;
                static std::shared_ptr<GpuMemoryHeap> readbackHeap_;
            };
        }
    }
}