#include "Submission.hpp"

#include "gapi_dx12/Device.hpp"

#include "common/debug/DebugStream.hpp"

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
        void Submission::putWork(T&& workVariant)
        {
            ASSERT(submissionThread_.IsJoinable())

            Work work;
            work.workVariant = std::move(workVariant);

#ifdef DEBUG
            constexpr int STACK_SIZE = 32;
            work.stackTrace.load_here(STACK_SIZE);
#endif
            inputWorkChannel_.Put(std::move(work));
        }

        Render::Result Submission::InitDevice()
        {
            return ExecuteAwait([](Render::Device& device) {
                return device.Init();
            });
        }

        void Submission::ExecuteAsync(CallbackFunction&& function)
        {
            Work::Callback work;
            work.function = function;

            putWork(work);
        }

        Render::Result Submission::ExecuteAwait(const CallbackFunction&& function)
        {
            Work::Callback work;

            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            std::condition_variable condition;
            Render::Result result = Render::Result::FAIL;

            work.function = [&condition, &function, &mutex, &result](Render::Device& device) {
                std::unique_lock<std::mutex> lock(mutex);
                result = function(device);
                condition.notify_one();
                return result;
            };

            putWork(work);

            condition.wait(lock);

            return result;
        }

        Render::Result Submission::ResetDevice(const PresentOptions& presentOptions)
        {
            return ExecuteAwait([&presentOptions](Render::Device& device) {
                return device.Reset(presentOptions);
            });
        }

        void Submission::Terminate()
        {
            ASSERT(submissionThread_.IsJoinable())

            Work::Terminate work;
            putWork(work);

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

                Render::Result result = Render::Result::FAIL;

                std::visit(overloaded {
                               [this, &result](const Work::Callback& work) { result = work.function(*device_); },
                               [this, &result](const Work::Terminate& work) {
                                   device_ == nullptr;
                                   result = Render::Result::OK;
                                   Log::Print::Info("Device terminated \n");
                               },
                               [](auto&& arg) { ASSERT_MSG(false, "Unsupported work type"); } },
                    inputWork.workVariant);

                std::this_thread::sleep_for(50ms);
            }
        }
    }
};
