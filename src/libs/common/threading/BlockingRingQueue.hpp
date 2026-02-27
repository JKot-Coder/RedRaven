#pragma once

#include "common/RingQueue.hpp"
#include "common/threading/Mutex.hpp"
#include <condition_variable>

namespace RR::Common::Threading
{
    template <typename T, std::size_t BufferSize>
    class BlockingRingQueue
    {
    public:
        BlockingRingQueue() = default;
        // Note: The queue should not be accessed concurrently while it's
        // being deleted. It's up to the user to synchronize this.
        ~BlockingRingQueue() = default;

        bool TryPush(const T& obj)
        {
            {
                Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);

                if (buffer.full())
                    return false;

                buffer.push_back(obj);
            }
            notEmpty.notify_one();
            return true;
        }

        bool TryPush(T&& obj)
        {
            {
                Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);

                if (buffer.full())
                    return false;

                buffer.push_back(std::move(obj));
            }
            notEmpty.notify_one();
            return true;
        }

        void Push(const T& obj)
        {
            {
                Threading::UniqueLock<Threading::Mutex> lock(mutex);
                notFull.wait(lock, [this]() { return !buffer.full(); });

                buffer.push_back(obj);
            }
            notEmpty.notify_one();
        }

        void Push(T&& obj)
        {
            {
                Threading::UniqueLock<Threading::Mutex> lock(mutex);
                notFull.wait(lock, [this]() { return !buffer.full(); });

                buffer.push_back(std::move(obj));
            }
            notEmpty.notify_one();
        }

        template <typename... Args>
        bool TryEmplace(Args&&... args)
        {
            {
                Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);

                if (buffer.full())
                    return false;

                buffer.emplace_back(std::forward<Args>(args)...);
            }
            notEmpty.notify_one();
            return true;
        }

        template <typename... Args>
        void Emplace(Args&&... args)
        {
            {
                Threading::UniqueLock<Threading::Mutex> lock(mutex);
                notFull.wait(lock, [this]() { return !buffer.full(); });

                buffer.emplace_back(std::forward<Args>(args)...);
            }
            notEmpty.notify_one();
        }

        bool TryPop(T& out)
        {
            {
                Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);

                if (buffer.empty())
                    return false;

                out = std::move(buffer.front());
                buffer.pop_front();
            }
            notFull.notify_one();
            return true;
        }

        T Pop()
        {
            Threading::UniqueLock<Threading::Mutex> lock(mutex);
            notEmpty.wait(lock, [this]() { return !buffer.empty(); });

            T temp = std::move(buffer.front());

            buffer.pop_front();

            lock.unlock();
            notFull.notify_one();

            return temp;
        }

        bool Empty() const
        {
            Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);
            return buffer.empty();
        }

        bool Full() const
        {
            Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);
            return buffer.full();
        }

        std::size_t Size() const
        {
            Threading::ReadWriteGuard<Threading::Mutex> lock(mutex);
            return buffer.size();
        }

    private:
        std::condition_variable notEmpty;
        std::condition_variable notFull;
        mutable Threading::Mutex mutex;
        RingQueue<T, BufferSize> buffer;
    };
}