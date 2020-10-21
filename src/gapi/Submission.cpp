#include "Submission.hpp"

#include "gapi_dx12/Device.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace std::chrono;

namespace OpenDemo
{
    namespace Render
    {
        Submission::Submission()
        {
        }

        Submission::~Submission()
        {
            ASSERT(!device_)
            ASSERT(!submissionThread_.IsJoinable())
            ASSERT(inputWorkChannel_.IsClosed())
        }

        void Submission::Start()
        {
            ASSERT(!submissionThread_.IsJoinable())
            ASSERT(!inputWorkChannel_.IsClosed())

            submissionThread_ = Threading::Thread("Submission Thread", [this] {
                device_.reset(new Render::DX12::Device());
                this->threadFunc();
            });
        }

        template <typename T>
        void Submission::PutWork(const T& work)
        {
            ASSERT(submissionThread_.IsJoinable())

            inputWorkChannel_.Put(work);
        }

        Render::Result Submission::InitDevice()
        {
            Render::Result result;

            ExecuteOnSubmission([&result](Render::Device& device) {
                result = device.Init();
            },
                true);

            return result;
        }

        void Submission::ExecuteOnSubmission(const CallbackFunction& function, bool waitForExcecution)
        {
            Work::Callback work;

            std::condition_variable condition;

            if (!waitForExcecution)
            {
                work.function = function;
                PutWork(work);
                return;
            }

            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);

            work.function = [&condition, &function, &mutex](Render::Device& device) {
                std::unique_lock<std::mutex> lock(mutex);
                function(device);
                condition.notify_one();
            };

            PutWork(work);

            condition.wait(lock);
        }

        void Submission::ResetDevice(const PresentOptions& presentOptions)
        {
            ExecuteOnSubmission([&presentOptions](Render::Device& device) {
                device.Reset(presentOptions);
            },
                true);
        }

        void Submission::Terminate()
        {
            ASSERT(submissionThread_.IsJoinable())

            Work::Terminate work;
            PutWork(work);

            inputWorkChannel_.Close();
            submissionThread_.Join();
        }

        // helper type for the visitor
        template <class... Ts>
        struct overloaded : Ts...
        {
            using Ts::operator()...;
        };
        // explicit deduction guide (not needed as of C++20)
        template <class... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;

        void Submission::threadFunc()
        {
            while (true)
            {
                auto inputWorkOptional = inputWorkChannel_.GetNext();
                // Channel was closed and no work
                if (!inputWorkOptional.has_value())
                    return;

                const auto& inputWork = inputWorkOptional.value();

                ASSERT(device_)

                std::visit(overloaded {
                               [this](const Work::Callback& work) { work.function(*device_); },
                               [this](const Work::Terminate& work) {
                                   device_ == nullptr;
                                   Log::Print::Info("Device terminated \n");
                               },
                               [](auto&& arg) { ASSERT_MSG(false, "Unsupported work type"); } },
                    inputWork);

                std::this_thread::sleep_for(50ms);
            }
        }
    }
};
