#pragma once

#include "gapi_dx12/FenceImpl.hpp"

#include "common/threading/Mutex.hpp"

#include <queue>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceReleaseContext
            {

            public:
                struct ResourceRelease
                {
                    uint64_t cpuFrameIndex;
                    ComSharedPtr<IUnknown> resource;
                };

            public:
                ResourceReleaseContext() = default;

                Result Init(const ComSharedPtr<ID3D12Device>& device)
                {
                    fence_ = std::make_unique<FenceImpl>();
                    D3DCall(fence_->Init(device, "ResourceRelease"));

                    return Result::Ok;
                }

                template <class T>
                void DeferredD3DResourceRelease(ComSharedPtr<T>& resource)
                {
                    Threading::ReadWriteGuard lock(mutex_);

                    ASSERT(fence_);
                    ASSERT(resource);

                    queue_.push({ fence_->GetCpuValue(), resource });
                    resource = nullptr;
                }

                Result ExecuteDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue)
                {
                    Threading::ReadWriteGuard lock(mutex_);

                    ASSERT(fence_);
                    ASSERT(queue);

                    while (queue_.size() && queue_.front().cpuFrameIndex < fence_->GetGpuValue())
                        queue_.pop();

                    D3DCall(fence_->Signal(*queue.get()));

                    return Result::Ok;
                }

            private:
                std::unique_ptr<FenceImpl> fence_;
                std::queue<ResourceRelease> queue_;
                Threading::Mutex mutex_;
            };

        }
    }
}