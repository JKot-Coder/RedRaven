#pragma once

#include "common/RingBuffer.hpp"
#include "common/Singleton.hpp"
#include "common/threading/SpinLock.hpp"

namespace RR
{
    namespace Common
    {
        class IDataBuffer;
    }

    namespace GAPI::DX12
    {
        class CommandListImpl;
        class CommandQueueImpl;
        class FenceImpl;
        class ResourceImpl;

        class InitialDataUploder final : public Singleton<InitialDataUploder>
        {
        public:
            InitialDataUploder();
            void Init();
            void Terminate();
            void DefferedUpload(const ResourceImpl& resource, const std::shared_ptr<IDataBuffer>& initialData);
            void FlushAndWaitFor(CommandQueueImpl& commandQueue);

        private:
            void submit();

        private:
            static constexpr size_t UploadBatchSize = 16;
        
        private:
            bool inited_ = false;

            std::unique_ptr<FenceImpl> fence_;
            std::unique_ptr<CommandQueueImpl> commandQueue_;
            std::unique_ptr<CommandListImpl> commandList_;
            size_t pendingDefferedUploads;

            Threading::SpinLock spinlock_;
        };
    }
}