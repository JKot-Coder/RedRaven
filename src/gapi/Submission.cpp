#include "Submission.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/Result.hpp"
#include "gapi/SwapChain.hpp"

#include "gapi_dx12/Device.hpp"

#include "common/debug/DebugStream.hpp"
#include "common/threading/BufferedChannel.hpp"
#include "common/threading/ConditionVariable.hpp"
#include "common/threading/Mutex.hpp"

#include <chrono>
#include <optional>
#include <thread>
#include <variant>

using namespace std::chrono;

namespace OpenDemo
{
    namespace Render
    {
        namespace
        {
            struct Work final
            {
            public:
                Work() = default;
                Work(Work&&) = default;

                struct Terminate
                {
                };

                struct Callback
                {
                    Submission::CallbackFunction function;
                };

                struct Submit
                {
                    std::shared_ptr<CommandContext> commandContext;
                };

                using WorkVariant = std::variant<Terminate, Callback, Submit>;

            public:
                WorkVariant workVariant;
#ifdef DEBUG
                backward::StackTrace stackTrace;
                U8String label;
#endif
            };
        }

        Submission::Submission()
            : inputWorkChannel_(std::make_unique<BufferedChannel>())
        {
        }

        Submission::~Submission()
        {
            ASSERT(device_)
            ASSERT(!submissionThread_.IsJoinable())
            ASSERT(inputWorkChannel_->IsClosed())
        }

        void Submission::Start()
        {
            ASSERT(!submissionThread_.IsJoinable())
            ASSERT(!inputWorkChannel_->IsClosed())

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
            inputWorkChannel_->Put(std::move(work));
        }

        void Submission::Submit(const CommandContext::SharedPtr& commandContext)
        {
            Work::Submit work;
            work.commandContext = commandContext;

            putWork(work);
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

            Threading::Mutex mutex;
            std::unique_lock<Threading::Mutex> lock(mutex);
            Threading::ConditionVariable condition;
            Render::Result result = Render::Result::Fail;

            work.function = [&condition, &function, &mutex, &result](Render::Device& device) {
                std::unique_lock<Threading::Mutex> lock(mutex);
                result = function(device);
                condition.notify_one();
                return result;
            };

            putWork(work);

            condition.wait(lock);

            return result;
        }

        void Submission::Terminate()
        {
            ASSERT(submissionThread_.IsJoinable())

            Work::Terminate work;
            putWork(work);

            inputWorkChannel_->Close();
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
                auto inputWorkOptional = inputWorkChannel_->GetNext();
                // Channel was closed and no work
                if (!inputWorkOptional.has_value())
                    return;

                const auto& inputWork = inputWorkOptional.value();

                ASSERT(device_)

                Render::Result result = Render::Result::Fail;

                std::visit(overloaded {
                               [this, &result](const Work::Submit& work) { result = device_->Submit(work.commandContext); },
                               [this, &result](const Work::Callback& work) { result = work.function(*device_); },
                               [this, &result](const Work::Terminate& work) {
                                   device_ == nullptr;
                                   result = Render::Result::Ok;
                                   Log::Print::Info("Device terminated.\n");
                               },
                               [](auto&& arg) { ASSERT_MSG(false, "Unsupported work type"); } },
                    inputWork.workVariant);

                // Todo: Add work label in message
                if (!result)
                    Log::Print::Fatal(u8"Fatal error on SubmissionThread with result: %s\n", result.ToString());

                std::this_thread::sleep_for(510ms);
            }
        }
    }
};
