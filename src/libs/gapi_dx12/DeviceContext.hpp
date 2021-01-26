#pragma once

#include "common/Singleton.hpp"

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

            class DeviceContext : public Singleton<DeviceContext>
            {
            public:
                void Init(const ComSharedPtr<ID3D12Device>& device,
                          const ComSharedPtr<IDXGIFactory2>& dxgiFactory)
                {
                    ASSERT(device);
                    ASSERT(dxgiFactory);

                    device_ = device;
                    dxgiFactory_ = dxgiFactory;
                }

                void Init(const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                          const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet,
                          const std::shared_ptr<ResourceReleaseContext>& resourceReleaseContext,
                          const std::shared_ptr<GpuMemoryHeap>& uploadHeap)
                {
                    ASSERT(descriptorHeapSet);
                    ASSERT(graphicsCommandQueue);
                    ASSERT(resourceReleaseContext);
                    ASSERT(uploadHeap);

                    graphicsCommandQueue_ = graphicsCommandQueue;
                    descriptorHeapSet_ = descriptorHeapSet;
                    resourceReleaseContext_ = resourceReleaseContext;
                    uploadHeap_ = uploadHeap;
                }

                void Terminate()
                {
                    device_ = nullptr;
                    dxgiFactory_ = nullptr;
                    graphicsCommandQueue_ = nullptr;
                    descriptorHeapSet_ = nullptr;
                    resourceReleaseContext_ = nullptr;
                }

                ComSharedPtr<ID3D12Device> GetDevice() const
                {
                    ASSERT(device_);
                    return device_;
                }

                ComSharedPtr<IDXGIFactory2> GetDxgiFactory() const
                {
                    ASSERT(dxgiFactory_);
                    return dxgiFactory_;
                }

                std::shared_ptr<CommandQueueImpl> GetGraphicsCommandQueue() const
                {
                    ASSERT(graphicsCommandQueue_);
                    return graphicsCommandQueue_;
                }

                std::shared_ptr<DescriptorHeapSet> GetDesciptorHeapSet() const
                {
                    ASSERT(descriptorHeapSet_);
                    return descriptorHeapSet_;
                }

                std::shared_ptr<ResourceReleaseContext> GetResourceReleaseContext() const
                {
                    ASSERT(resourceReleaseContext_);
                    return resourceReleaseContext_;
                }

                std::shared_ptr<GpuMemoryHeap> GetUploadHeap() const
                {
                    ASSERT(uploadHeap_);
                    return uploadHeap_;
                }

            private:
                ComSharedPtr<ID3D12Device> device_;
                ComSharedPtr<IDXGIFactory2> dxgiFactory_;
                std::shared_ptr<CommandQueueImpl> graphicsCommandQueue_;
                std::shared_ptr<DescriptorHeapSet> descriptorHeapSet_;
                std::shared_ptr<ResourceReleaseContext> resourceReleaseContext_;
                std::shared_ptr<GpuMemoryHeap> uploadHeap_;
            };
        }
    }
}