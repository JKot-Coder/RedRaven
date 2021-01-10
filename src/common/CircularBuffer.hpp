#pragma once

namespace OpenDemo
{
    namespace Common
    {
        template <typename T, std::size_t Capacity>
        class CircularBuffer
        {
            using Pointer = T*;

            static_assert(Capacity > 0);

        public:
            CircularBuffer() : front_(&buffer_[0]), back_(&buffer_[0]), size_(0) { }

            inline void push_back(const T& item)
            {
                if (full())
                    throw std::out_of_range("CircularBuffer is full");

                *back_ = item;
                increment(back_);
                ++size_;
            }

            template <typename... Args>
            inline void emplace_back(Args&&... args)
            {
                push_back(T(std::forward<Args>(args)...));
            }

            void pop_front()
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");

                --size_;
                increment(front_);
            }

            T& front()
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");
                return *front_;
            }

            inline const T& front() const
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");
                return *front_;
            }

            inline const T& back() const
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");
                return *back_;
            }

            inline size_t capacity() const { return Capacity; }
            inline size_t size() const { return size_; }
            inline bool empty() const { return size_ == 0; }
            inline bool full() const { return size_ == Capacity; }

        private:
            void increment(Pointer& ptr)
            {
                ++ptr;

                if (ptr == &buffer_[0] + Capacity)
                    ptr = &buffer_[0];
            }

        private:
            std::array<T, Capacity> buffer_;
            Pointer front_;
            Pointer back_;
            size_t size_ = 0;
        };
    }
}