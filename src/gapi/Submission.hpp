#pragma once

#include "common/AccessGuard.hpp"
#include "common/CircularBuffer.hpp"

#include "gapi/DeviceInterface.hpp"

#include <optional>
#include <queue>

namespace OpenDemo
{
    namespace Render
    {
        class Submission
        {
            // Queue of 64 work should be enough.
            static constexpr inline size_t WorkBufferSize = 64;

        public:
            Submission();

            void Start();

        private:
            void ThreadFunc();

        private:
            template <typename T, std::size_t BufferSize>
            class BufferedChannel
            {
            public:
                BufferedChannel() = default;
                ~BufferedChannel() = default;

                inline void Put(const T& obj)
                {
                    if (!closed_)
                        return;

                    std::unique_lock lock(mutex_);

                    if (buffer_.full())
                    {
                        outputWait_.wait(lock, [&]() { return !buffer_.full() || closed_; });
                        if (closed_)
                            return;
                    }

                    buffer_.push(T);
                    inputWait_.notify_one();
                }

                inline std::optional<T> GetNext() const
                {
                    std::unique_lock lock(mutex_);

                    if (buffer_.empty())
                    {
                        if (closed_)
                            return std::nullopt;

                        inputWait_.wait(lock, [&]() { return !buffer_.empty() || closed_; });

                        if (closed_)
                            return std::nullopt;
                    }

                    T temp;
                    std::swap(temp, buffer.front());

                    std::optional<T> item = buffer_.pop_front();
                    outputWait_.notify_one();
                    return temp;
                }

                inline std::optional<T> TryGetNext() const
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
                std::atomic_bool closed_ = false;
                std::condition_variable inputWait_;
                std::condition_variable outputWait_;
                std::mutex mutex_;
                CircularBuffer<T, BufferSize> buffer_;
            };

        private:
            std::unique_ptr<AccessGuard<Render::Device>> device_;
            std::thread submissionThread_;
            BufferedChannel<int, WorkBufferSize> inputWorkChannel_;
        };

    }
}