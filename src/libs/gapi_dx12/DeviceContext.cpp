#include "DeviceContext.hpp"

#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            D3D12MA::Allocator* DeviceContext::allocator_;
            ComSharedPtr<ID3D12Device> DeviceContext::device_;
            ComSharedPtr<IDXGIFactory2> DeviceContext::dxgiFactory_;
            std::shared_ptr<CommandQueueImpl> DeviceContext::graphicsCommandQueue_;
            std::shared_ptr<DescriptorHeapSet> DeviceContext::descriptorHeapSet_;
            std::shared_ptr<ResourceReleaseContext> DeviceContext::resourceReleaseContext_;
            std::shared_ptr<GpuMemoryHeap> DeviceContext::uploadHeap_;
            std::shared_ptr<GpuMemoryHeap> DeviceContext::readbackHeap_;

            void DeviceContext::Init(const ComSharedPtr<ID3D12Device>& device,
                                     const ComSharedPtr<IDXGIFactory2>& dxgiFactory)
            {
                ASSERT(device);
                ASSERT(dxgiFactory);

                device_ = device;
                dxgiFactory_ = dxgiFactory;
            }

            void DeviceContext::Init(D3D12MA::Allocator* allocator,
                                     const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                                     const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet,
                                     const std::shared_ptr<ResourceReleaseContext>& resourceReleaseContext,
                                     const std::shared_ptr<GpuMemoryHeap>& uploadHeap,
                                     const std::shared_ptr<GpuMemoryHeap>& readbackHeap)
            {
                ASSERT(allocator);
                ASSERT(descriptorHeapSet);
                ASSERT(graphicsCommandQueue);
                ASSERT(resourceReleaseContext);
                ASSERT(uploadHeap);
                ASSERT(readbackHeap);

                allocator_ = allocator;
                graphicsCommandQueue_ = graphicsCommandQueue;
                descriptorHeapSet_ = descriptorHeapSet;
                resourceReleaseContext_ = resourceReleaseContext;
                uploadHeap_ = uploadHeap;
                readbackHeap_ = readbackHeap;
            }

            void DeviceContext::Terminate()
            {
                allocator_->Release();
                allocator_ = nullptr;
                device_ = nullptr;
                dxgiFactory_ = nullptr;
                graphicsCommandQueue_ = nullptr;
                descriptorHeapSet_ = nullptr;
                resourceReleaseContext_ = nullptr;
                uploadHeap_ = nullptr;
                readbackHeap_ = nullptr;
            }

        }
    }
}