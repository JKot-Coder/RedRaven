#include "Submission.hpp"

#include <chrono>
#include <thread>

using namespace std::chrono;

namespace OpenDemo
{
    namespace Render
    {
        template <>
        void Submission::PutWork<Submission::Work::Type::InitDevice>()
        {
            inputWorkChannel_.Put(Work::Create<Submission::Work::Type::InitDevice>());
        }

        template <>
        void Submission::PutWork<Submission::Work::Type::Terminate>()
        {
            inputWorkChannel_.Put(Work::Create<Submission::Work::Type::Terminate>());
        }

        void Submission::Start()
        {
            submissionThread_ = std::thread([this] { this->threadFunc(); });
            PutWork<Work::Type::InitDevice>();
        }

        template <>
        void Submission::process<Submission::Work::Type::InitDevice>()
        {
            Log::Print::Info("InitDevice \n");
        }

        template <>
        void Submission::process<Submission::Work::Type::Terminate>()
        {
            Log::Print::Info("Terminate \n");

            inputWorkChannel_.Close();
            submissionThread_.join();
        }

        void Submission::threadFunc()
        {
            while (true)
            {
                auto inputWorkOptional = inputWorkChannel_.GetNext();
                // Channel was closed
                if (!inputWorkOptional.has_value())
                    return;

#define CASE_WORK(type)              \
    case Work::Type::type:           \
        process<Work::Type::type>(); \
        break;

                const auto& inputWork = inputWorkOptional.value();
                switch (inputWork.GetType())
                {
                    CASE_WORK(InitDevice)
                    CASE_WORK(Terminate)
                default:
                    ASSERT_MSG(false, "Unsupported work type");
                    break;
                }
#undef CASE_WORK

                // Log::Print::Info("%d\n", inputWork);
                std::this_thread::sleep_for(50ms);
            }
        }

    }
};
