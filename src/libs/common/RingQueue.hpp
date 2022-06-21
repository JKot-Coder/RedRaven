#pragma once

namespace RR
{
    namespace Common
    {
        template <typename T, std::size_t SIZE>
        class RingQueue
        {
            using Pointer = T*;
            static_assert(SIZE > 0);

        public:
            typedef typename std::array<T, SIZE>::value_type value_type;
            typedef typename std::array<T, SIZE>::reference reference;
            typedef typename std::array<T, SIZE>::const_reference const_reference;
            typedef typename std::array<T, SIZE>::size_type size_type;

            RingQueue() : front_(&buffer_[0]), back_(&buffer_[0]), size_(0) { }
            ~RingQueue()
            {
                while (!empty())
                    pop_front();
            }

            inline void push_back(const T& item)
            {
                if (full())
                    throw std::out_of_range("RingQueue is full");

                *back_ = item;
                increment(back_);
                ++size_;
            }

            template <typename... Args>
            inline void emplace_back(Args&&... args)
            {
                if (full())
                    throw std::out_of_range("RingQueue is full");

                new (&back()) T(std::forward<Args>(args)...);
                increment(back_);
                ++size_;
            }

            void pop_front()
            {
                if (empty())
                    throw std::out_of_range("RingQueue is empty");

                front().~T();
                --size_;
                increment(front_);
            }

            T& front()
            {
                if (empty())
                    throw std::out_of_range("RingQueue is empty");
                return *front_;
            }

            inline const T& front() const
            {
                if (empty())
                    throw std::out_of_range("RingQueue is empty");
                return *front_;
            }

            inline const T& back() const
            {
                if (empty())
                    throw std::out_of_range("RingQueue is empty");
                return *back_;
            }

            inline size_t capacity() const { return SIZE; }
            inline size_t size() const { return size_; }
            inline bool empty() const { return size_ == 0; }
            inline bool full() const { return size_ == SIZE; }

        private:
            void increment(Pointer& ptr)
            {
                ++ptr;

                if (ptr == &buffer_[0] + SIZE)
                    ptr = &buffer_[0];
            }

        private:
            std::array<T, SIZE> buffer_;
            Pointer front_;
            Pointer back_;
            size_t size_ = 0;
        };
    }
}