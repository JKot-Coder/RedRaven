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

                void Init()
                {
                    fence_ = std::make_unique<FenceImpl>();
                    fence_->Init("ResourceRelease");
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

                void ExecuteDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue)
                {
                    Threading::ReadWriteGuard lock(mutex_);

                    ASSERT(fence_);
                    ASSERT(queue);

                    while (queue_.size() && queue_.front().cpuFrameIndex < fence_->GetGpuValue())
                        queue_.pop();

                    fence_->Signal(*queue.get());
                }

            private:
                std::unique_ptr<FenceImpl> fence_;
                std::queue<ResourceRelease> queue_;
                Threading::Mutex mutex_;
            };
        }
    }
}