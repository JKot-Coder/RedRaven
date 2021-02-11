#pragma once

#include <mutex>
#include <shared_mutex>

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            // Todo Threading::Mutex
            template <typename T>
            class AccessGuard
            {
                // std::shared_mutex does not guarantee that there will be no problems with unique look starvation. But we will hope the best.
                // Might be it would be better to use this shared mutex https://github.com/AlexeyAB/object_threadsafe/blob/master/contfree_shared_mutex/safe_ptr.h
                using SharedMutex = std::shared_mutex;

            public:
                template <typename... Args>
                AccessGuard(Args... args) : ptr_(std::make_shared<T>(args...)), mutex_(std::make_shared<SharedMutex>()) { }

                ~AccessGuard() = default;

                std::shared_ptr<T>& operator=(const std::shared_ptr<T>& r) noexcept
                {
                    ptr_ = r;
                    return ptr_;
                }

                std::shared_ptr<T>& operator=(std::shared_ptr<T>&& r) noexcept
                {
                    ptr_ = std::move(r);
                    return ptr_;
                }

            private:
                template <typename LockType>
                class Pointer
                {
                public:
                    Pointer() = delete;

                    Pointer(const std::shared_ptr<T>& ptr, const std::shared_ptr<SharedMutex>& mutex) : ptr_(ptr), mutex_(mutex), lock_(*mutex) {};
                    Pointer(Pointer&& other) : ptr_(other.ptr_), mutex_(other.mutex_), lock_(other.lock_) {};

                    ~Pointer() = default;

                    inline operator bool() const { return ptr_.operator bool(); };
                    inline T* operator->() { return ptr_.get(); }
                    inline const T* operator->() const { return ptr_.get(); }

                private:
                    LockType lock_;
                    std::shared_ptr<SharedMutex> mutex_;
                    std::shared_ptr<T> ptr_;
                };

            public:
                using ExclusiveAcessPointer = Pointer<std::unique_lock<SharedMutex>>;
                using SharedAcessPointer = Pointer<std::shared_lock<SharedMutex>>;

            public:
                inline ExclusiveAcessPointer ExclusiveAcess() { return ExclusiveAcessPointer(ptr_, mutex_); }
                inline SharedAcessPointer SharedAcess() { return SharedAcessPointer(ptr_, mutex_); }
                inline const ExclusiveAcessPointer ExclusiveAcess() const { return ExclusiveAcessPointer(ptr_, mutex_); }
                inline const SharedAcessPointer SharedAcess() const { return SharedAcessPointer(ptr_, mutex_); }

            private:
                std::shared_ptr<SharedMutex> mutex_;
                std::shared_ptr<T> ptr_;
            };
        }
    }
}