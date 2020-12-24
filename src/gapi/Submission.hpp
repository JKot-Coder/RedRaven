#pragma once

#include "gapi/DeviceInterface.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

#include "common/threading/Thread.hpp"

#undef NOMINMAX
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#include "dependencies/backward-cpp/backward.hpp"
#pragma warning(pop)
#define NOMINMAX

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
            struct Work;
        }

        class Submission final
        {
        public:
            using CallbackFunction = std::function<Render::Result(Render::Device& device)>;

            Submission();
            ~Submission();

            void Start();
            void Terminate();
            void Submit(const std::shared_ptr<CommandQueue>& commandQueue, const std::shared_ptr<CommandList>& commandList);

            void ExecuteAsync(CallbackFunction&& function);
            Render::Result ExecuteAwait(const CallbackFunction&& function);

            inline std::weak_ptr<Render::MultiThreadDeviceInterface> GetMultiThreadDeviceInterface() { return device_; }

        private:
            template <typename T>
            void putWork(T&& work);

            void threadFunc();

        private:
            // Queue of 64 work should be enough.
            static constexpr size_t WorkBufferSize = 64;
            using BufferedChannel = Threading::BufferedChannel<Work, WorkBufferSize>;

            std::shared_ptr<Render::Device> device_;
            //   std::unique_ptr<AccessGuard<Render::Device>> device_;
            Threading::Thread submissionThread_;
            std::unique_ptr<BufferedChannel> inputWorkChannel_;
        };

    }
}