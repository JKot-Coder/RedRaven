#pragma once

#include "common/CircularBuffer.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
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

                    std::unique_lock lock(mutex_);

                    if (buffer_.full())
                    {
                        outputWait_.wait(lock, [&]() { return !buffer_.full() || closed_; });
                        if (closed_)
                            return;
                    }

                    buffer_.push(obj);
                    inputWait_.notify_one();
                }

                inline std::optional<T> GetNext()
                {
                    std::unique_lock lock(mutex_);

                    if (buffer_.empty())
                    {
                        if (closed_)
                            return std::nullopt;

                        inputWait_.wait(lock, [&]() { return !buffer_.empty() || closed_; });

                        if (buffer_.empty() && closed_)
                            return std::nullopt;
                    }

                    const auto& item = buffer_.pop_front();
                    outputWait_.notify_one();
                    return item;
                }

                inline std::optional<T> TryGetNext()
                {
                    std::unique_lock lock(mutex_);

                    if (buffer_.empty())
                        return std::nullopt;

                    const auto& item = buffer_.pop_front();
                    outputWait_.notify_one();
                    return item;
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
                std::mutex mutex_;
                CircularBuffer<T, BufferSize> buffer_;
            };

        }
    }
}