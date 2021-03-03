#pragma once

#include "common/threading/Mutex.hpp"

#include <queue>

namespace D3D12MA
{
    class Allocation;
}

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class FenceImpl;
            class CommandQueueImpl;

            class ResourceReleaseContext final
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

                template <class T>
                void DeferredD3DResourceRelease(ComSharedPtr<T>& resource, D3D12MA::Allocation* allocation = nullptr)
                {
                    deferredD3DResourceRelease(resource.as<IUnknown>(), allocation);
                    resource = nullptr;
                }

                void ExecuteDeferredDeletions(const std::shared_ptr<CommandQueueImpl>& queue);

            private:
                void deferredD3DResourceRelease(const ComSharedPtr<IUnknown>& resource, D3D12MA::Allocation* allocation);

            private:
                std::unique_ptr<FenceImpl> fence_;
                std::queue<ResourceRelease> queue_;
                Threading::Mutex mutex_;
            };
        }
    }
}