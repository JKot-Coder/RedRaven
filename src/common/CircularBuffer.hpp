#pragma once

#include <array>

namespace OpenDemo
{
    namespace Common
    {

        template <typename T, std::size_t Capacity>
        class CircularBuffer
        {
            using Pointer = T*;

        public:
            CircularBuffer() : front_(&buffer_[0]), back_(&buffer_[0]), size_(0) { }

            void push(T item) { push_back(item); }

            inline void push_back(T item)
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

            void pop()
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");

                --size_;
                increment(front_);
            }

            T pop_front()
            {
                if (empty())
                    throw std::out_of_range("CircularBuffer is empty");

                --size_;
                T temp = *front_;
                increment(front_);
                return temp;
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

            inline size_t size() const { return size_; }
            inline bool empty() const { return size_ == 0; }
            inline bool full() const { return size_ == Capacity; }

        private:
            void increment(Pointer& ptr) const
            {
                ++ptr;

                if (ptr == &buffer_[0] + Capacity)
                    ptr = &m_buffer[0];
            }

        private:
            std::array<T, Capacity> buffer_;
            Pointer front_;
            Pointer back_;
            size_t size_ = 0;
        };
    }
}