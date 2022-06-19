#pragma once

#include "common/Singleton.hpp"
#include "common/threading/SpinLock.hpp"

#include <queue>

#include "gapi_dx12/D3DUtils/D3DUtils.hpp"

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

                bool IsInited() const { return inited_; }

                template <class T>
                void static DeferredD3DResourceRelease(ComSharedPtr<T>& resource, D3D12MA::Allocation* allocation = nullptr)
                {
                    auto& instance = Instance();
                    if (!instance.IsInited())
                    {
                        Log::Format::Warning("Resource of type {} has been leaked.\n", D3DUtils::GetTypeName<T>());
                        resource.detach();
                        return;
                    }

                    instance.deferredD3DResourceRelease(resource.as<IUnknown>(), allocation);
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
                bool inited_ = false;
                std::unique_ptr<FenceImpl> fence_;
                std::queue<ResourceRelease> queue_;
                Threading::SpinLock spinlock_;
            };
        }
    }
}