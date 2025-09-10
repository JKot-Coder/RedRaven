#include "Submission.hpp"

#include <chrono>
#include <thread>

using namespace std::chrono;

namespace RR::Render
{
    Submission::Submission() : submissionEvent(false, false) { }

    void Submission::Start(GAPI::Device::UniquePtr device, SubmissionThreadMode mode)
    {
        ASSERT(device);
        this->device = eastl::move(device);

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

    void Submission::ExecuteAwait(SubmissionThreadFunction&& function)
    {
        SubmissionThreadWork work = SubmissionThreadWork::Callback(eastl::move(function));

        if (!isSubmissionThread())
        {
            workQueue.Emplace(eastl::move(work));
            blockUntilFinished();
        }
        else
            doWork(work);
    }

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

    bool Submission::doWork(const SubmissionThreadWork& work)
    {
        switch (work.GetType())
        {
            case SubmissionThreadWork::Type::Terminate:
                return true;
            case SubmissionThreadWork::Type::Callback:
                work.function(*device);
                break;
            default:
                ASSERT_MSG(false, "Invalid work type");
                break;
        }

        return false;
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