#pragma once

#include "common/threading/ConditionVariable.hpp"
#include "common/threading/Mutex.hpp"

namespace RR
{
    namespace Common
    {
        namespace Threading
        {
            const uint32_t INFINITE_WAIT = 0xFFFFFFFF;

            class Event final
            {
            public:
                NONCOPYABLE_MOVABLE(Event);
                Event(bool manualReset = true, bool initialState = false) : state_(initialState), manualReset_(manualReset) { }
                ~Event() = default;

                inline void Reset()
                {
                    UniqueLock<Mutex> lock(mutex_);
                    state_ = false;
                }

                inline void Notify()
                {
                    {
                        UniqueLock<Mutex> lock(mutex_);
                        state_ = true;
                    }

                    condition_.notify_all();
                }

                inline bool Wait(uint32_t milliseconds = INFINITE_WAIT)
                {
                    UniqueLock<Mutex> lock(mutex_);
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