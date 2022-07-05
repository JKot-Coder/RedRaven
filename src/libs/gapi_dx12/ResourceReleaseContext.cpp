#include "ResourceReleaseContext.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

#include "common/threading/Mutex.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            ResourceReleaseContext::~ResourceReleaseContext()
            {
                Threading::ReadWriteGuard lock(spinlock_);
                ASSERT_MSG(!inited_, "Resource release context is alive, did you call Terminate()?");
                ASSERT(queue_.size() == 0);
            }

            void ResourceReleaseContext::Init()
            {
                ASSERT(!inited_);

                fence_ = std::make_unique<FenceImpl>();
                fence_->Init("ResourceRelease");

                inited_ = true;
            }

            void ResourceReleaseContext::Terminate()
            {
                ASSERT(inited_);

                const auto gpuFenceValue = fence_->GetGpuValue();
                while (!queue_.empty())
                {
                    ASSERT(queue_.front().cpuFrameIndex >= gpuFenceValue);

                    auto& allocation = queue_.front().allocation;

                    if (allocation)
                        allocation->Release();

                    allocation = nullptr;

                    queue_.pop();
                }

                inited_ = false;
                fence_ = nullptr;
            }

            void ResourceReleaseContext::deferredD3DResourceRelease(const ComSharedPtr<IUnknown>& resource, D3D12MA::Allocation* allocation)
            {
                ASSERT(inited_);

                if (!resource)
                    return;

                Threading::ReadWriteGuard lock(spinlock_);
                queue_.push({ fence_->GetCpuValue(), resource, allocation });
            }

            void ResourceReleaseContext::executeDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue)
            {
                Threading::ReadWriteGuard lock(spinlock_);

                ASSERT(inited_);
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