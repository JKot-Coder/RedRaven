#pragma once

#include "common/RingBuffer.hpp"
#include "common/Singleton.hpp"
#include "common/threading/Mutex.hpp"

#include <future>
#include <queue>

namespace RR::GAPI
{
    class GpuResource;
    class CommandQueue;
    class CpuResourceData;
    class CopyCommandList;
}

namespace RR::Render
{
    class GpuResourceTransfer final : public Singleton<GpuResourceTransfer>
    {
    public:
        void Init();
        void Terminate();

        void PerformTransfers(const std::shared_ptr<GAPI::CommandQueue>& commandQueue);

        std::shared_future<void> DefferedUpload(const std::shared_ptr<GAPI::GpuResource>& resource, const std::shared_ptr<GAPI::CpuResourceData>& data);

    private:
        void resetTransferTask();

    private:
        bool inited_ = false;
        uint32_t defferedCommmandsCount_ = 0;

        Threading::Mutex mutex_;
        RingBuffer<std::shared_ptr<GAPI::CopyCommandList>, 2> commandLists_;
        std::packaged_task<void(const std::shared_ptr<GAPI::CommandQueue>&, const std::shared_ptr<GAPI::CopyCommandList>&)> defferedTransferTask_;
        std::shared_future<void> defferedTransferFuture_;
    };
}