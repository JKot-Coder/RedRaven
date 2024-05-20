#include "Submission.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/SwapChain.hpp"

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
#pragma warning(disable : 26495)
//#include "dependencies/backward-cpp/backward.hpp"
#pragma warning(pop)
#define NOMINMAX

using namespace std::chrono;

namespace RR
{
    namespace Render
    {
        namespace
        {
            bool isListTypeCompatable(GAPI::CommandQueueType commandQueueType, GAPI::CommandListType commandListType)
            {
                return (commandQueueType == GAPI::CommandQueueType::Copy && commandListType == GAPI::CommandListType::Copy) ||
                       (commandQueueType == GAPI::CommandQueueType::Compute && commandListType == GAPI::CommandListType::Compute) ||
                       (commandQueueType == GAPI::CommandQueueType::Graphics && commandListType == GAPI::CommandListType::Graphics);
            }

            struct Task final
            {
            public:
                Task() = default;

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
                // backward::StackTrace stackTrace;
                // U8String label;
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
        }

        void Submission::Start(const GAPI::Device::SharedPtr& device)
        {
            ASSERT(device);
            ASSERT(!inputTaskChannel_->IsClosed());

            device_ = device;

#if ENABLE_SUBMISSION_THREAD
            ASSERT(!submissionThread_.IsJoinable());

            submissionThread_ = Threading::Thread("Submission Thread", [this]
                                                  { this->threadFunc(); });
#endif
        }

        bool Submission::isSubmissionThread()
        {
#if ENABLE_SUBMISSION_THREAD
            return std::this_thread::get_id() == submissionThread_.GetId();
#else
            return true;
#endif
        }

        template <>
        inline void Submission::doTask(const Task::Submit& task)
        {
            task.commandQueue->Submit(task.commandList);
        }

        template <>
        inline void Submission::doTask(const Task::Callback& task)
        {
            task.function(*device_);
        }

        template <>
        inline void Submission::doTask(const Task::Terminate& task)
        {
            std::ignore = task;
            device_.reset();
            Log::Print::Info("Device terminated.\n");
        }

        template <typename T>
        void Submission::putTask(T&& taskVariant)
        {
#if ENABLE_SUBMISSION_THREAD
            ASSERT(submissionThread_.IsJoinable())

            Task task;
            task.taskVariant = std::move(taskVariant);

#ifdef DEBUG
            //  constexpr int STACK_SIZE = 32;
            //  task.stackTrace.load_here(STACK_SIZE);
#endif

            inputTaskChannel_->Put(task);
#else
            ASSERT(device_);
            doTask(taskVariant);
#endif
        }

        void Submission::Submit(const GAPI::CommandQueue::SharedPtr& commandQueue, const GAPI::CommandList::SharedPtr& commandList)
        {
            ASSERT(commandQueue);
            ASSERT(commandList);
            ASSERT(isListTypeCompatable(commandQueue->GetCommandQueueType(), commandList->GetCommandListType()));

            Task::Submit task;
            task.commandQueue = commandQueue;
            task.commandList = commandList;

            if (!isSubmissionThread())
            {
                putTask(task);
            }
            else
                doTask(task);
        }

        void Submission::ExecuteAsync(const CallbackFunction&& function)
        {
            //  Task::Callback task;
            // task.function = function;

            putTask(Task::Callback { function });
        }

        void Submission::ExecuteAwait(const CallbackFunction&& function)
        {
            Task::Callback task;

#if ENABLE_SUBMISSION_THREAD
            Threading::Mutex mutex;
            Threading::UniqueLock<Threading::Mutex> lock(mutex);
            Threading::ConditionVariable condition;

            if (!isSubmissionThread())
            {
                task.function = [&condition, &function, &mutex](GAPI::Device& device)
                {
                    Threading::UniqueLock<Threading::Mutex> lock(mutex);
                    function(device);
                    condition.notify_one();
                };

                putTask(task);

                condition.wait(lock);
            }
            else
#endif
            {
                task.function = function;
                doTask(task);
            }
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

            //Reset channel to allow reuse submission after terminate
            inputTaskChannel_ = std::make_unique<BufferedChannel>();
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

                std::visit(
                    overloaded {
                        [this](const Task::Submit& task)
                        { return doTask(task); },
                        [this](const Task::Callback& task)
                        { return doTask(task); },
                        [this](const Task::Terminate& task)
                        { return doTask(task); },
                    },
                    inputTask.taskVariant);

                //std::this_thread::sleep_for(50ms);
            }
        }
#endif
    }
};