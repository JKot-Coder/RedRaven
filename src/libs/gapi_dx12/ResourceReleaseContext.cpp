#include "ResourceReleaseContext.hpp"

#include "common/threading/Mutex.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            ResourceReleaseContext::~ResourceReleaseContext()
            {
                Threading::ReadWriteGuard lock(spinlock_);
                ASSERT_MSG(!fence_, "Fence is still alive, did you call Terminate()?");
                ASSERT(queue_.size() == 0);
            }

            void ResourceReleaseContext::Init()
            {
                fence_ = std::make_unique<FenceImpl>();
                fence_->Init("ResourceRelease");
            }

            void ResourceReleaseContext::Terminate()
            {
                fence_ = nullptr;
            }

            void ResourceReleaseContext::deferredD3DResourceRelease(const ComSharedPtr<IUnknown>& resource, D3D12MA::Allocation* allocation)
            {
                if (!resource)
                    return;

                //Resource might be leaked. Ignore it
                if (!fence_)
                    return;
                    
                Threading::ReadWriteGuard lock(spinlock_);
                queue_.push({ fence_->GetCpuValue(), resource, allocation });
            }

            void ResourceReleaseContext::executeDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue)
            {
                Threading::ReadWriteGuard lock(spinlock_);

                ASSERT(fence_);
                ASSERT(queue);

                const auto gpuFenceValue = fence_->GetGpuValue();
                while (queue_.size() && queue_.front().cpuFrameIndex < gpuFenceValue)
                {
                    auto& allocation = queue_.front().allocation;

                    if (allocation)
                        allocation->Release();

                    allocation = nullptr;

                    queue_.pop();
                }

                fence_->Signal(*queue.get());
            }
        }
    }
}