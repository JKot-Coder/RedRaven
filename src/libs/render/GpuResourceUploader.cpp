#include "GpuResourceUploader.hpp"

#include "DeviceContext.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"

namespace RR::Render
{
    namespace
    {
        void submitCommandList(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CopyCommandList>& commandList)
        {
            auto& deviceContext = DeviceContext::Instance();

            deviceContext.Submit(commandQueue, commandList);
        }
    }

    void GpuResourceTransfer::Init()
    {
        Threading::ReadWriteGuard lock(mutex_);

        ASSERT(!inited_);

        if (inited_)
            return;

        auto& deviceContext = DeviceContext::Instance();

        commandLists_ = { deviceContext.CreateGraphicsCommandList("GpuResourceTransfer_1"),
                          deviceContext.CreateGraphicsCommandList("GpuResourceTransfer_2") };
        resetTransferTask();

        inited_ = true;
    }

    void GpuResourceTransfer::Terminate()
    {
        Threading::ReadWriteGuard lock(mutex_);

        ASSERT(inited_);

        if (!inited_)
            return;

        inited_ = false;
    }

    void GpuResourceTransfer::resetTransferTask()
    {
        defferedTransferTask_ = std::packaged_task(&submitCommandList);
        defferedTransferFuture_ = defferedTransferTask_.get_future();
    }

    void GpuResourceTransfer::PerformTransfers(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
    {
        Threading::ReadWriteGuard lock(mutex_);

        ASSERT(inited_);

        if (defferedCommmandsCount_ == 0)
            return;

        commandLists_.Peek()->Close();
        defferedCommmandsCount_ = 0;

        defferedTransferTask_(commandQueue, commandLists_.Peek());
        commandLists_.Advance();

        resetTransferTask();
    }

    std::shared_future<void> GpuResourceTransfer::DefferedUpload(const std::shared_ptr<GAPI::GpuResource>& resource, const std::shared_ptr<GAPI::CpuResourceData>& data)
    {
        Threading::ReadWriteGuard lock(mutex_);

        ASSERT(inited_);

        defferedCommmandsCount_++;
        commandLists_.Peek()->UpdateGpuResource(resource, data);
        return defferedTransferFuture_;
    }
}