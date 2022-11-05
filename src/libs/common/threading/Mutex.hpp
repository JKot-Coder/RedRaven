#pragma once

#include <mutex>

namespace RR::Common
{
    namespace Threading
    {
        // Type aliasing
        using Mutex = std::mutex;
        using RecursiveMutex = std::recursive_mutex;

        template <class T>
        using UniqueLock = std::unique_lock<T>;

        template <class Lock>
        class ReadWriteGuard final : NonCopyable
        {
        public:
            ReadWriteGuard() = delete;

            ReadWriteGuard(Lock& lock) : lock_(&lock)
            {
                lock_->lock();
            }

            ~ReadWriteGuard()
            {
                if (lock_)
                    lock_->unlock();
            }

            ReadWriteGuard(ReadWriteGuard&& other) : lock_(other.lock_)
            {
                other.lock_ = nullptr;
            }

            ReadWriteGuard& operator=(ReadWriteGuard&& other)
            {
                lock_ = other.lock_;
                other.lock_ = nullptr;
                return *this;
            }

        private:
            Lock* lock_;
        };
    }
}