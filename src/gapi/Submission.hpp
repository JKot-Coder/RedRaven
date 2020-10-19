#pragma once

#include "common/AccessGuard.hpp"
#include "common/CircularBuffer.hpp"

#include "gapi/DeviceInterface.hpp"

#include <atomic>
#include <optional>
#include <queue>

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
                Work() = default;

                enum class Type
                {
                    Udefined,
                    InitDevice,
                    Terminate
                };

                template <Work::Type type>
                static inline Work Create();

                template <>
                static inline Work Create<Work::Type::InitDevice>()
                {
                    return Work(InitWorkData {});
                }

                template <>
                static inline Work Create<Work::Type::Terminate>()
                {
                    return Work(TerminateWorkData {});
                }

                Type GetType() const { return type_; }

            private:
                struct InitWorkData
                {
                };

                struct TerminateWorkData
                {
                };

            private:
                Work(const InitWorkData& data) : type_(Work::Type::InitDevice), initWorkData_(data) {};
                Work(const TerminateWorkData& data) : type_(Work::Type::Terminate), terminateWordData_(data) {};

            private:
                Type type_ = Type::Udefined;

                union
                {
                    InitWorkData initWorkData_;
                    TerminateWorkData terminateWordData_;
                };
            };

        public:
            Submission() = default;

            void Start();

        private:
            template <Work::Type type>
            void PutWork();

        public:
            template <typename T, std::size_t BufferSize>
            class BufferedChannel
            {
            public:
                static_assert(std::is_trivially_move_constructible<T>::value);

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

                        if (closed_)
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
            template <Work::Type type>
            void process();

            void threadFunc();

        private:
            std::unique_ptr<AccessGuard<Render::Device>> device_;
            std::thread submissionThread_;
            BufferedChannel<Work, WorkBufferSize> inputWorkChannel_;
        };

    }
}