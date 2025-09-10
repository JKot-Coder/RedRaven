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
            Callback
        };

    public:
        SubmissionThreadWork() = default;
        SubmissionThreadWork(Type type) : type(type) { }
        SubmissionThreadWork(SubmissionThreadFunction&& function) :  type(Type::Callback), function(eastl::move(function)) { }
        ~SubmissionThreadWork() = default;

        Type GetType() const { return type; }
    public:
        static SubmissionThreadWork Terminate()
        {
            return SubmissionThreadWork(Type::Terminate);
        }

        static SubmissionThreadWork Callback(SubmissionThreadFunction&& function)
        {
            return SubmissionThreadWork(eastl::move(function));
        }

    private:
        friend class Submission;

        Type type;
        SubmissionThreadFunction function;
    };

    class Submission final
    {
    public:
        Submission();
        ~Submission();

        void Start(GAPI::Device::UniquePtr device, SubmissionThreadMode mode);

        bool isSubmissionThread() const { return submissionThread ? (std::this_thread::get_id() == submissionThread->GetId()) : true; }

        void ExecuteAwait(SubmissionThreadFunction&& function);
        void Terminate();

    private:
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