#pragma once

#include "common/RingQueue.hpp"
#include "common/threading/Mutex.hpp"

#include <atomic>
#include <optional>

namespace RR::Common::Threading
{
    template <typename T, std::size_t BufferSize>
    class BufferedChannel
    {
    public:
        BufferedChannel() = default;
        ~BufferedChannel() = default;

        inline void Put(const T& obj)
        {
            if (closed_)
                return;

            Threading::UniqueLock<Threading::Mutex> lock(mutex_);

            if (buffer_.full())
            {
                outputWait_.wait(lock, [&]() { return !buffer_.full() || closed_; });
                if (closed_)
                    return;
            }

            buffer_.push_back(obj);
            inputWait_.notify_one();
        }

        inline std::optional<T> GetNext()
        {
            Threading::UniqueLock<Threading::Mutex> lock(mutex_);

            if (buffer_.empty())
            {
                if (closed_)
                    return std::nullopt;

                inputWait_.wait(lock, [&]() { return !buffer_.empty() || closed_; });

                if (buffer_.empty() && closed_)
                    return std::nullopt;
            }

            const auto temp = std::make_optional<T>(std::move(buffer_.front()));
            buffer_.pop_front();
            outputWait_.notify_one();

            return std::move(temp);
        }

        inline std::optional<T> TryGetNext()
        {
            Threading::UniqueLock<Threading::Mutex> lock(mutex_);

            if (buffer_.empty())
                return std::nullopt;

            auto temp = std::make_optional<T>(std::move(buffer_.front()));
            buffer_.pop_front();
            outputWait_.notify_one();

            return std::move(temp);
        }

        inline void Close()
        {
            closed_ = true;
            inputWait_.notify_all();
            outputWait_.notify_all();
        }

        inline bool IsClosed() const { return closed_; }

    private:
        std::atomic<bool> closed_ = false;
        std::condition_variable inputWait_;
        std::condition_variable outputWait_;
        Threading::Mutex mutex_;
        RingQueue<T, BufferSize> buffer_;
    };
}