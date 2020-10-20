#pragma once

#include "common/AccessGuard.hpp"
#include "common/CircularBuffer.hpp"

#include "gapi/DeviceInterface.hpp"

#include <atomic>
#include <optional>
#include <queue>
#include <variant>

namespace OpenDemo
{
    namespace Render
    {
        class Submission final
        {
            // Queue of 64 work should be enough.
            static constexpr inline size_t WorkBufferSize = 64;

        public:
            struct Work final
            {
            public:
                struct Terminate
                {
                };

                struct Callback
                {
                    std::function<void()> function;
                };
            };

            using WorkVariant = std::variant<Work::Terminate, Work::Callback>;

        public:
            Submission();
            ~Submission();

            void Start();
            Render::Result InitDevice();
            void ExecuteOnSubmission(const std::function<void()>& function, bool waitForExcecution);
            void Terminate();

        private:
            template <typename T>
            void PutWork(const T& work);

        public:
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

        private:
            void threadFunc();

        private:
            std::unique_ptr<Render::Device> device_;
            //   std::unique_ptr<AccessGuard<Render::Device>> device_;
            std::thread submissionThread_;
            BufferedChannel<WorkVariant, WorkBufferSize> inputWorkChannel_;
        };

    }
}