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

        void Submission::Start()
        {
            submissionThread_ = std::thread([this] { this->threadFunc(); });
        }

        template <typename T>
        void Submission::PutWork(const T& data)
        {
            const Work work(data);
            inputWorkChannel_.Put(work);
        }

        Render::Result Submission::InitDevice()
        {
            Render::Result result;

            ExecuteOnSubmission([this, &result] {
                device_.reset(new Render::DX12::Device());
                result = device_->Init();
            },
                true);

            return result;
        }

        void Submission::ExecuteOnSubmission(const std::function<void()>& function, bool waitForExcecution)
        {
            Work::CallbackData data;

            std::condition_variable condition;

            if (!waitForExcecution)
            {
                data.function = function;
                PutWork(data);
                return;
            }

            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);

            data.function = [&condition, &function, &mutex] {
                std::unique_lock<std::mutex> lock(mutex);
                function();
                condition.notify_one();
            };

            PutWork(data);

            condition.wait(lock);
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
                // Channel was closed
                if (!inputWorkOptional.has_value())
                    return;

                const auto& inputWork = inputWorkOptional.value();

                std::visit(overloaded {
                               [](const Submission::Work::InitData& data) { Log::Print::Info("InitDevice \n"); },
                               [](const Submission::Work::CallbackData& data) { data.function(); },
                               [this](const Submission::Work::TerminateData& data) {
                                   inputWorkChannel_.Close();
                                   submissionThread_.join();
                               },
                               [](auto&& arg) { ASSERT_MSG(false, "Unsupported work type"); } },
                    inputWork.data);

                std::this_thread::sleep_for(50ms);
            }
        }
    }
};
