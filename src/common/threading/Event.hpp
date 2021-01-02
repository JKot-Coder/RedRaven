#pragma once

#include "common/threading/ConditionVariable.hpp"
#include "common/threading/Mutex.hpp"

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            const uint32_t INFINITE_WAIT = 0xFFFFFFFF;

            class Event final : private NonCopyable, NonMovable
            {
            public:
                Event(bool manualReset = true, bool initialState = false) : manualReset_(manualReset), state_(initialState) { }
                ~Event() = default;

                inline void Reset()
                {
                    std::unique_lock<Mutex> lock(mutex_);
                    state_ = false;
                }

                inline void Notify()
                {
                    {
                        std::unique_lock<Mutex> lock(mutex_);
                        state_ = true;
                    }

                    condition_.notify_all();
                }

                inline bool Wait(uint32_t milliseconds = INFINITE_WAIT)
                {
                    std::unique_lock<Mutex> lock(mutex_);
                    bool result = true;

                    if (state_ == false)
                    {
                        if (milliseconds == INFINITE_WAIT)
                        {
                            condition_.wait(lock, [&]() { return state_; });
                        }
                        else
                        {
                            result = condition_.wait_for(lock, std::chrono::milliseconds(milliseconds), [&]() { return state_; });
                        }
                    }

                    if (!manualReset_)
                        state_ = false;

                    return result;
                }

            private:
                Mutex mutex_;
                ConditionVariable condition_;
                bool state_ = false;
                bool manualReset_ = true;
            };
        }
    }
}