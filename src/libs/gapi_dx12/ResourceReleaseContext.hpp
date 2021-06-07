#pragma once

#include "common/Singleton.hpp"
#include "common/threading/SpinLock.hpp"

#include <queue>

namespace D3D12MA
{
    class Allocation;
}

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            class FenceImpl;
            class CommandQueueImpl;

            class ResourceReleaseContext final : public Singleton<ResourceReleaseContext>
            {
            public:
                struct ResourceRelease
                {
                    uint64_t cpuFrameIndex;
                    ComSharedPtr<IUnknown> resource;
                    D3D12MA::Allocation* allocation;
                };

            public:
                ResourceReleaseContext() = default;
                ~ResourceReleaseContext();

                void Init();
                void Terminate();

                template <class T>
                void static DeferredD3DResourceRelease(ComSharedPtr<T>& resource, D3D12MA::Allocation* allocation = nullptr)
                {
                    Instance().deferredD3DResourceRelease(resource.as<IUnknown>(), allocation);
                    resource = nullptr;
                }

                void static ExecuteDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue)
                {
                    Instance().executeDeferredDeletions(queue);
                }

            private:
                void deferredD3DResourceRelease(const ComSharedPtr<IUnknown>& resource, D3D12MA::Allocation* allocation);
                void executeDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue);

            private:
                std::unique_ptr<FenceImpl> fence_;
                std::queue<ResourceRelease> queue_;
                Threading::SpinLock spinlock_;
            };
        }
    }
}