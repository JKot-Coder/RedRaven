#pragma once

#include "gapi/Device.hpp"

#include "common/threading/Thread.hpp"

#define ENABLE_SUBMISSION_THREAD true


namespace RR::Render
{
    class Submission final
    {
    public:
        Submission(GAPI::Device::UniquePtr device);
        ~Submission();

        void Start();
        void Terminate();

    private:
#if ENABLE_SUBMISSION_THREAD
        void threadFunc();
#endif

    private:
        GAPI::Device::UniquePtr device;
#if ENABLE_SUBMISSION_THREAD
        Threading::Thread submissionThread;
#endif
    };
}