#include "Submission.hpp"

#include <chrono>
#include <thread>

#include "gapi/CommandQueue.hpp"
#include "gapi/CommandList.hpp"

using namespace std::chrono;

namespace RR::Render
{
    Submission::Submission() : submissionEvent(false, false) { }

    void Submission::Start(GAPI::Device::UniquePtr devicePtr, SubmissionThreadMode mode)
    {
        ASSERT(devicePtr);
        device = eastl::move(devicePtr);

        if (mode == SubmissionThreadMode::Enabled)
        {
            ASSERT(!submissionThread || !submissionThread->IsJoinable());
            submissionThread = eastl::make_unique<Threading::Thread>("Submission Thread", [this] { this->threadFunc(); });
        }
    }

    Submission::~Submission()
    {
        ASSERT(!device);
        ASSERT(!submissionThread);
    };

    void Submission::Terminate()
    {
        if (submissionThread)
        {
            workQueue.Emplace(SubmissionThreadWork::Terminate());

            ASSERT(submissionThread->IsJoinable());
            submissionThread->Join();
            submissionThread.reset();
        }

        device.reset();
    }

    void Submission::ExecuteAsync(SubmissionThreadFunction&& function)
    {
        SubmissionThreadWork work = SubmissionThreadWork::Callback(eastl::move(function));
        executeAsync(eastl::move(work));
    }

    void Submission::ExecuteAwait(SubmissionThreadFunction&& function)
    {
        SubmissionThreadWork work = SubmissionThreadWork::Callback(eastl::move(function));
        executeAwait(eastl::move(work));
    }

    void Submission::Submit(GAPI::CommandQueue* commandQueue, GAPI::CommandList2& commandList)
    {
        ASSERT(commandQueue);

        SubmissionThreadWork work = SubmissionThreadWork::Submit(*commandQueue, commandList);
        executeAsync(eastl::move(work));
    }

    void Submission::Present(GAPI::SwapChain* swapChain)
    {
        ASSERT(swapChain);

        SubmissionThreadWork work = SubmissionThreadWork::Present(swapChain);
        executeAsync(eastl::move(work));
    }

    void Submission::MoveToNextFrame(uint64_t frameIndex)
    {
        SubmissionThreadWork work = SubmissionThreadWork::MoveToNextFrame(frameIndex);
        executeAsync(eastl::move(work));
    }

    void Submission::executeAsync(SubmissionThreadWork&& work)
    {
        if (!isSubmissionThread())
            workQueue.Emplace(eastl::move(work));
        else
            doWork(work);
    }

    void Submission::executeAwait(SubmissionThreadWork&& work)
    {
        if (!isSubmissionThread())
        {
            workQueue.Emplace(eastl::move(work));
            blockUntilFinished();
        }
        else
            doWork(work);
    }

    void Submission::blockUntilFinished()
    {
        if(!submissionThread)
            return;

        ASSERT(!isSubmissionThread());

        Common::Threading::ReadWriteGuard<Common::Threading::Mutex> lock(mutex);

        SubmissionThreadWork notifyWork = SubmissionThreadWork::Callback([this](GAPI::Device&) {
            submissionEvent.Notify();
        });

        workQueue.Emplace(eastl::move(notifyWork));

        // Wait for the notification
        submissionEvent.Wait();
    }

    bool Submission::doWork(const SubmissionThreadWork& work)
    {
        switch (work.GetType())
        {
            case SubmissionThreadWork::Type::Terminate:
                return true;
            case SubmissionThreadWork::Type::Callback:
                work.function(*device);
                break;
            case SubmissionThreadWork::Type::MoveToNextFrame:
                device->MoveToNextFrame(work.frameIndex);
                break;
            case SubmissionThreadWork::Type::Present:
                device->Present(work.swapChain);
                break;
            case SubmissionThreadWork::Type::Submit:
                work.submit.commandQueue->Submit(*work.submit.commandList);
                break;
            default:
                ASSERT_MSG(false, "Invalid work type");
                break;
        }

        return false;
    }

    void Submission::threadFunc()
    {
        bool quit = false;
        while (!quit)
        {
            auto work = workQueue.Pop();
            quit = doWork(work);
        }
    }
}