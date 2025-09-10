#pragma once

#include "gapi/Device.hpp"

#include "common/threading/Thread.hpp"
#include "common/threading/BlockingRingQueue.hpp"
#include "common/threading/Event.hpp"

#include <EASTL/fixed_function.h>

namespace RR::Render
{
    enum class SubmissionThreadMode
    {
        Enabled,
        Disabled
    };

    static constexpr size_t SubmissionThreadFunctionSize = 64;
    using SubmissionThreadFunction = eastl::fixed_function<SubmissionThreadFunctionSize, void(GAPI::Device&)>;

    struct SubmissionThreadWork final
    {
        enum class Type
        {
            Terminate,
            Callback,
            Present,
            Submit,
            MoveToNextFrame
        };

    public:
        SubmissionThreadWork() = default;
        ~SubmissionThreadWork() = default;

        Type GetType() const { return type; }

    private:
        SubmissionThreadWork(Type type) : type(type) { }
        SubmissionThreadWork(SubmissionThreadFunction&& function) : type(Type::Callback), function(eastl::move(function)) { }
        SubmissionThreadWork(uint64_t frameIndex) : type(Type::MoveToNextFrame), frameIndex(frameIndex) { }
        SubmissionThreadWork(GAPI::SwapChain* swapChain) : type(Type::Present), swapChain(swapChain) { }
        SubmissionThreadWork(GAPI::CommandQueue& commandQueue, GAPI::CommandList2& commandList) : type(Type::Submit), submit {&commandQueue, &commandList} { }

    public:
        static SubmissionThreadWork Terminate()
        {
            return SubmissionThreadWork(Type::Terminate);
        }

        static SubmissionThreadWork Callback(SubmissionThreadFunction&& function)
        {
            return SubmissionThreadWork(eastl::move(function));
        }

        static SubmissionThreadWork MoveToNextFrame(uint64_t frameIndex)
        {
            return SubmissionThreadWork(frameIndex);
        }

        static SubmissionThreadWork Present(GAPI::SwapChain* swapChain)
        {
            return SubmissionThreadWork(swapChain);
        }

        static SubmissionThreadWork Submit(GAPI::CommandQueue& commandQueue, GAPI::CommandList2& commandList)
        {
            return SubmissionThreadWork(commandQueue, commandList);
        }

    private:
        friend class Submission;

        Type type;
        SubmissionThreadFunction function;
        struct Submit
        {
            GAPI::CommandQueue* commandQueue;
            GAPI::CommandList2* commandList;
        };

        union
        {
            uint64_t frameIndex;
            GAPI::SwapChain* swapChain;
            struct Submit submit;
        };
    };

    class Submission final
    {
    public:
        Submission();
        ~Submission();

        bool isSubmissionThread() const { return submissionThread ? (std::this_thread::get_id() == submissionThread->GetId()) : true; }

        void Start(GAPI::Device::UniquePtr device, SubmissionThreadMode mode);
        void Terminate();

        void ExecuteAsync(SubmissionThreadFunction&& function);
        void ExecuteAwait(SubmissionThreadFunction&& function);
        void Submit(GAPI::CommandQueue* commandQueue, GAPI::CommandList2& commandList);
        void Present(GAPI::SwapChain* swapChain);
        void MoveToNextFrame(uint64_t frameIndex);

    private:
        void executeAsync(SubmissionThreadWork&& work);
        void executeAwait(SubmissionThreadWork&& work);

        bool doWork(const SubmissionThreadWork& work);
        void blockUntilFinished();
        void threadFunc();

    private:
        GAPI::Device::UniquePtr device;
        eastl::unique_ptr<Threading::Thread> submissionThread;
        Common::Threading::Event submissionEvent;
        Common::Threading::Mutex mutex;
        Common::Threading::BlockingRingQueue<SubmissionThreadWork, 64> workQueue;
    };
}