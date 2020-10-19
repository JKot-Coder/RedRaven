#include "Submission.hpp"

namespace OpenDemo
{
    namespace Render
    {
        void Submission::Start()
        {
            submissionThread_ = std::thread([this] { this->ThreadFunc(); });
        }

        void Submission::ThreadFunc()
        {
            while(true)
            { 
                auto inputWork = inputWorkChannel_.GetNext();
                // No work returned if channel is closed/
                ASSERT(inputWork.has_value());

                Log::Print::Info("%d\n", inputWork.value());
            }
        }

    }
};
