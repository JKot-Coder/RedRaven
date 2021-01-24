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

            class DeviceContext : public Singleton<DeviceContext>
            {
            public:
                void Init(const ComSharedPtr<ID3D12Device>& device,
                          const ComSharedPtr<IDXGIFactory2>& dxgiFactory,
                          const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                          const std::shared_ptr<DescriptorHeapSet>& descriptorHeapSet,
                          const std::shared_ptr<ResourceReleaseContext>& resourceReleaseContext)
                {
                    ASSERT(device);
                    ASSERT(dxgiFactory);
                    ASSERT(descriptorHeapSet);
                    ASSERT(graphicsCommandQueue);
                    ASSERT(resourceReleaseContext);

                    device_ = device;
                    dxgiFactory_ = dxgiFactory;
                    graphicsCommandQueue_ = graphicsCommandQueue;
                    descriptorHeapSet_ = descriptorHeapSet;
                    resourceReleaseContext_ = resourceReleaseContext;
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
                    ASSERT(device_)
                    return device_;
                }

                ComSharedPtr<IDXGIFactory2> GetDxgiFactory() const
                {
                    ASSERT(dxgiFactory_)
                    return dxgiFactory_;
                }

                std::shared_ptr<CommandQueueImpl> GetGraphicsCommandQueue() const
                {
                    ASSERT(graphicsCommandQueue_)
                    return graphicsCommandQueue_;
                }

                std::shared_ptr<DescriptorHeapSet> GetDesciptorHeapSet() const
                {
                    ASSERT(descriptorHeapSet_)
                    return descriptorHeapSet_;
                }

                std::shared_ptr<ResourceReleaseContext> GetResourceReleaseContext() const
                {
                    ASSERT(resourceReleaseContext_)
                    return resourceReleaseContext_;
                }

            private:
                ComSharedPtr<ID3D12Device> device_;
                ComSharedPtr<IDXGIFactory2> dxgiFactory_;
                std::shared_ptr<CommandQueueImpl> graphicsCommandQueue_;
                std::shared_ptr<DescriptorHeapSet> descriptorHeapSet_;
                std::shared_ptr<ResourceReleaseContext> resourceReleaseContext_;
            };
        }
    }
}