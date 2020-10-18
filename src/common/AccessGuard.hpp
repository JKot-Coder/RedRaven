#pragma once

#include <mutex>
#include <shared_mutex>

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

private:
    template <typename LockType>
    class AccessGuardPointer
    {
    public:
        AccessGuardPointer() = delete;

        AccessGuardPointer(const std::shared_ptr<T>& ptr, const std::shared_ptr<SharedMutex>& mutex) : ptr_(ptr), mutex_(mutex), lock_(*mutex) {};
        AccessGuardPointer(AccessGuardPointer&& other) : ptr_(other.ptr_), mutex_(other.mutex_), lock_(other.lock_) {};

        ~AccessGuardPointer() = default;

        T* operator->() { return ptr_.get(); }
        const T* operator->() const { return ptr_.get(); }

    private:
        LockType lock_;
        std::shared_ptr<SharedMutex> mutex_;
        std::shared_ptr<T> ptr_;
    };

public:
    using ExclusiveAcessPointer = AccessGuardPointer<std::unique_lock<SharedMutex>>;
    using SharedAcessPointer = AccessGuardPointer<std::shared_lock<SharedMutex>>;

public:
    ExclusiveAcessPointer ExclusiveAcess() { return ExclusiveAcessPointer(ptr_, mutex_); }
    SharedAcessPointer SharedAcess() { return SharedAcessPointer(ptr_, mutex_); }
    const ExclusiveAcessPointer ExclusiveAcess() const { return ExclusiveAcessPointer(ptr_, mutex_); }
    const SharedAcessPointer SharedAcess() const { return SharedAcessPointer(ptr_, mutex_); }

private:
    std::shared_ptr<SharedMutex> mutex_;
    std::shared_ptr<T> ptr_;
};