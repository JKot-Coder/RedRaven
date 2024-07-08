#pragma once

#include <thread>

#ifdef OS_WINDOWS
#include <windows.h>
#endif // OS_WINDOWS

namespace RR
{
    namespace Common
    {
        namespace Threading
        {
            class Thread : NonCopyable
            {
            public:
                Thread() = default;
                ~Thread() = default;
                Thread(Thread&& other) noexcept : thread_(std::move(other.thread_)) {};

                template <class Function, class... Args>
                explicit Thread(const std::string& threadName, Function&& f, Args&&... args) : thread_(f, args...) { SetName(threadName); }

                Thread& operator=(Thread&& other) noexcept
                {
                    thread_.operator=(std::move(other.thread_));
                    return *this;
                }

                inline void SetName(const std::string& threadName)
                {
#ifdef OS_WINDOWS
                    const auto& wThreadName = StringConversions::UTF8ToWString(threadName);
                    SetThreadDescription(static_cast<HANDLE>(GetNativeHandle()), wThreadName.c_str());
#else
                    ASSERT_MSG(false, "Not implemented");
#endif
                }

                inline bool IsJoinable() const noexcept
                {
                    return thread_.joinable();
                }

                inline std::thread::id GetId() const noexcept
                {
                    return thread_.get_id();
                }

                inline std::thread::native_handle_type GetNativeHandle()
                {
                    return thread_.native_handle();
                }

                inline void Join()
                {
                    thread_.join();
                }

                inline void Detach()
                {
                    thread_.detach();
                }

                inline void swap(Thread& other) noexcept
                {
                    thread_.swap(other.thread_);
                }

                inline static unsigned int HardwareConcurrency() noexcept
                {
                    return std::thread::hardware_concurrency();
                }

            private:
                std::thread thread_;
            };
        }
    }
}