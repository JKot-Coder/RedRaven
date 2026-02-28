#pragma once

#include "common/threading/Mutex.hpp"
#include "common/NonCopyableMovable.hpp"
#include <limits>
#include <cstdint>
#include <chrono>
#include <condition_variable>

namespace RR::Common::Threading
{
    constexpr uint32_t INFINITE_WAIT = std::numeric_limits<uint32_t>::max();

    class Event final : public Common::NonCopyableMovable
    {
    public:
        explicit Event(bool manualReset = true, bool initialState = false) : state_(initialState), manualReset_(manualReset) { }
        ~Event() = default;

        void Reset() noexcept
        {
            ReadWriteGuard<Mutex> lock(mutex_);
            state_ = false;
        }

        void Notify() noexcept
        {
            {
                ReadWriteGuard<Mutex> lock(mutex_);
                state_ = true;
            }

            condition_.notify_all();
        }

        bool Wait(uint32_t milliseconds = INFINITE_WAIT)
        {
            UniqueLock<Mutex> lock(mutex_);
            bool result = true;

            if (!state_)
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
        std::condition_variable condition_;
        bool state_ = false;
        bool manualReset_ = true;
    };
}