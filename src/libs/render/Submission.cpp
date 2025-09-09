#include "Submission.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono;

namespace RR::Render
{
    Submission::Submission(GAPI::Device::UniquePtr device) : device(eastl::move(device))
    {
        ASSERT(device);
    }

    void Submission::Start()
    {
#if ENABLE_SUBMISSION_THREAD
        ASSERT(!submissionThread.IsJoinable());
        submissionThread = Threading::Thread("Submission Thread", [this] { this->threadFunc(); });
#endif
    }

    Submission::~Submission()
    {
        ASSERT(!device);
#if ENABLE_SUBMISSION_THREAD
        ASSERT(!submissionThread.IsJoinable());
#endif
    };

    void Submission::Terminate()
    {
    }

#if ENABLE_SUBMISSION_THREAD
    void Submission::threadFunc()
    {
        while (true)
        {
            std::this_thread::sleep_for(50ms);
        }
    }
#endif
}