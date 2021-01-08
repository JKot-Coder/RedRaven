#include "Submission.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
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

#undef NOMINMAX
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#include "dependencies/backward-cpp/backward.hpp"
#pragma warning(pop)
#define NOMINMAX

using namespace std::chrono;

namespace OpenDemo
{
    namespace Render
    {
        namespace
        {
            struct Task final
            {
            public:
                Task() = default;
                Task(Task&&) = default;

                struct Terminate
                {
                };

                struct Callback
                {
                    Submission::CallbackFunction function;
                };

                struct Submit
                {
                    std::shared_ptr<GAPI::CommandQueue> commandQueue;
                    std::shared_ptr<GAPI::CommandList> commandList;
                };

                using TaskVariant = std::variant<Terminate, Callback, Submit>;

            public:
                TaskVariant taskVariant;
#ifdef DEBUG
                backward::StackTrace stackTrace;
                U8String label;
#endif
            };
        }

        Submission::Submission()
            : inputTaskChannel_(std::make_unique<BufferedChannel>())
        {
        }

        Submission::~Submission()
        {
            ASSERT(!device_)
#if ENABLE_SUBMISSION_THREAD
            ASSERT(!submissionThread_.IsJoinable())
#endif
            ASSERT(inputTaskChannel_->IsClosed())
        }

        void Submission::Start()
        {
            ASSERT(!inputTaskChannel_->IsClosed())

#if ENABLE_SUBMISSION_THREAD
            ASSERT(!submissionThread_.IsJoinable())

            submissionThread_ = Threading::Thread("Submission Thread", [this] {
                device_.reset(new GAPI::DX12::Device());
                this->threadFunc();
            });
#else
            device_.reset(new GAPI::DX12::Device());
#endif
        }

        template <typename T>
        void Submission::putTask(T&& taskVariant)
        {
#if ENABLE_SUBMISSION_THREAD
            ASSERT(submissionThread_.IsJoinable())

            Task task;
            task.taskVariant = std::move(taskVariant);

#ifdef DEBUG
            constexpr int STACK_SIZE = 32;
            task.stackTrace.load_here(STACK_SIZE);
#endif

            inputTaskChannel_->Put(std::move(task));
#else
            ASSERT(device_);
            doTask(taskVariant);
#endif
        }

        void Submission::Submit(const GAPI::CommandQueue::SharedPtr& commandQueue, const GAPI::CommandList::SharedPtr& commandList)
        {
            Task::Submit task;
            task.commandQueue = commandQueue;
            task.commandList = commandList;

            putTask(task);
        }

        void Submission::ExecuteAsync(const CallbackFunction&& function)
        {
            Task::Callback task;
            task.function = function;

            putTask(task);
        }

        GAPI::Result Submission::ExecuteAwait(const CallbackFunction&& function)
        {
            Task::Callback task;

#if ENABLE_SUBMISSION_THREAD
            Threading::Mutex mutex;
            Threading::UniqueLock<Threading::Mutex> lock(mutex);
            Threading::ConditionVariable condition;
            GAPI::Result result = GAPI::Result::Fail;

            task.function = [&condition, &function, &mutex, &result](GAPI::Device& device) {
                Threading::UniqueLock<Threading::Mutex> lock(mutex);
                result = function(device);
                condition.notify_one();
                return result;
            };

            putTask(task);

            condition.wait(lock);
#else
            GAPI::Result result = GAPI::Result::Fail;

            task.function = [&function, &result](GAPI::Device& device) {
                result = function(device);
                return result;
            };

            putTask(task);
#endif

            return result;
        }

        void Submission::Terminate()
        {
            Task::Terminate task;
            putTask(task);

            inputTaskChannel_->Close();

#if ENABLE_SUBMISSION_THREAD
            ASSERT(submissionThread_.IsJoinable())
            submissionThread_.Join();
#endif
        }

        template <>
        inline GAPI::Result Submission::doTask(const Task::Submit& task)
        {
            return task.commandQueue->Submit(task.commandList);
        }

        template <>
        inline GAPI::Result Submission::doTask(const Task::Callback& task)
        {
            return task.function(*device_);
        }

        template <>
        inline GAPI::Result Submission::doTask(const Task::Terminate& task)
        {
            device_.reset();
            Log::Print::Info("Device terminated.\n");
            return GAPI::Result::Ok;
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

#if ENABLE_SUBMISSION_THREAD
        void Submission::threadFunc()
        {
            while (true)
            {
                auto inputTaskOptional = inputTaskChannel_->GetNext();
                // Channel was closed and no task
                if (!inputTaskOptional.has_value())
                    return;

                const auto& inputTask = inputTaskOptional.value();

                ASSERT(device_)

                GAPI::Result result = GAPI::Result::Fail;

                std::visit(overloaded {
                               [this, &result](const Task::Submit& task) { result = doTask(task); },
                               [this, &result](const Task::Callback& task) { result = doTask(task); },
                               [this, &result](const Task::Terminate& task) { result = doTask(task); },
                               [](auto&& arg) { ASSERT_MSG(false, "Unsupported task type"); } },
                    inputTask.taskVariant);

                // Todo: Add task label in message
                if (!result)
                    Log::Print::Fatal(u8"Fatal error on SubmissionThread with result: %s\n", result.ToString());

                //std::this_thread::sleep_for(50ms);
            }
        }
#endif
    }
};