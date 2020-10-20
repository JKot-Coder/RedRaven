#include "Submission.hpp"

#include "gapi_dx12/Device.hpp"

#include <chrono>
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
            ASSERT(!submissionThread_.joinable())
            ASSERT(inputWorkChannel_.IsClosed())
        }

        void Submission::Start()
        {
            ASSERT(!submissionThread_.joinable())
            ASSERT(!inputWorkChannel_.IsClosed())

            submissionThread_ = std::thread([this] {
                device_.reset(new Render::DX12::Device());
                this->threadFunc();
            });
        }

        template <typename T>
        void Submission::PutWork(const T& work)
        {
            ASSERT(submissionThread_.joinable())

            inputWorkChannel_.Put(work);
        }

        Render::Result Submission::InitDevice()
        {
            Render::Result result;

            ExecuteOnSubmission([this, &result] {
                result = device_->Init();
            },
                true);

            return result;
        }

        void Submission::ExecuteOnSubmission(const std::function<void()>& function, bool waitForExcecution)
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

            work.function = [&condition, &function, &mutex] {
                std::unique_lock<std::mutex> lock(mutex);
                function();
                condition.notify_one();
            };

            PutWork(work);

            condition.wait(lock);
        }

        void Submission::Terminate()
        {
            ASSERT(submissionThread_.joinable())

            Work::Terminate work;
            PutWork(work);

            inputWorkChannel_.Close();
            submissionThread_.join();
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
                               [](const Work::Callback& work) { work.function(); },
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
