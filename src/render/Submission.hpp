#pragma once

#include "gapi/DeviceInterface.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

#include "common/threading/Thread.hpp"

#define ENABLE_SUBMISSION_THREAD true

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            template <typename T, std::size_t BufferSize>
            class BufferedChannel;
        }
    }

    namespace Render
    {
        namespace
        {
            struct Task;
        }

        class Submission final
        {
        public:
            using CallbackFunction = std::function<GAPI::Result(GAPI::Device& device)>;

            Submission();
            ~Submission();

            void Start();
            void Terminate();
            void Submit(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList);

            void ExecuteAsync(CallbackFunction&& function);
            GAPI::Result ExecuteAwait(const CallbackFunction&& function);

            inline std::weak_ptr<GAPI::MultiThreadDeviceInterface> GetMultiThreadDeviceInterface() { return device_; }

        private:
            template <typename T>
            void putTask(T&& task);

            template <typename T>
            inline GAPI::Result doTask(const T& task);

#if ENABLE_SUBMISSION_THREAD
            void threadFunc();
#endif
        private:
            // Queue of 64 task should be enough.
            static constexpr size_t TaskBufferSize = 64;
            using BufferedChannel = Threading::BufferedChannel<Task, TaskBufferSize>;

            std::shared_ptr<GAPI::Device> device_;
            //   std::unique_ptr<AccessGuard<GAPI::Device>> device_;
#if ENABLE_SUBMISSION_THREAD
            Threading::Thread submissionThread_;
#endif
            std::unique_ptr<BufferedChannel> inputTaskChannel_;
        };

    }
}