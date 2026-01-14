#pragma once

#include <thread>

namespace RR::Common::Threading
{
    class Thread : public Common::NonCopyable
    {
    public:
        Thread() = default;
        ~Thread() = default;
        Thread(Thread&& other) noexcept : thread_(std::move(other.thread_)) {};

        template <class Function, class... Args>
        explicit Thread(const std::string& threadName, Function&& f, Args&&... args)
            : thread_(eastl::forward<Function>(f), eastl::forward<Args>(args)...)
        {
            setName(threadName);
        }

        Thread& operator=(Thread&& other) noexcept
        {
            thread_.operator=(std::move(other.thread_));
            return *this;
        }

        bool IsJoinable() const noexcept { return thread_.joinable(); }
        std::thread::id GetId() const noexcept { return thread_.get_id(); }
        std::thread::native_handle_type GetNativeHandle() { return thread_.native_handle(); }
        void Join() { thread_.join(); }
        void Detach() { thread_.detach(); }
        void swap(Thread& other) noexcept { thread_.swap(other.thread_); }
        static unsigned int HardwareConcurrency() noexcept { return std::thread::hardware_concurrency(); }

    private:
        void setName(const std::string& threadName);

    private:
        std::thread thread_;
    };
}