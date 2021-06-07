#pragma once

#include <atomic>
#include <thread>

namespace RR
{
    namespace Common
    {
        namespace Threading
        {
            class SpinLock final : private NonCopyable, NonMovable
            {
            public:
                SpinLock();
                ~SpinLock();

                // Lockable requirements compatible
                void lock();
                void unlock();
                bool tryLock();

            private:
                std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
            };

            inline SpinLock::SpinLock() { }

            inline SpinLock::~SpinLock() { }

            inline void SpinLock::lock()
            {
                while (flag_.test_and_set(std::memory_order_acquire))
                    std::this_thread::yield();
            }

            inline void SpinLock::unlock()
            {
                flag_.clear(std::memory_order_release);
            }

            inline bool SpinLock::tryLock()
            {
                return !flag_.test_and_set(std::memory_order_acquire);
            }
        }
    }
}