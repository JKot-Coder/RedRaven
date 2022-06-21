#pragma once

namespace RR::Common
{
    template <typename T, size_t SIZE>
    class RingBuffer
    {
    public:
        static_assert(SIZE >= 2);

        typedef typename std::array<T, SIZE>::value_type value_type;
        typedef typename std::array<T, SIZE>::reference reference;
        typedef typename std::array<T, SIZE>::const_reference const_reference;
        typedef typename std::array<T, SIZE>::size_type size_type;

        template <typename... Args>
        RingBuffer(Args&&... args) : buffer_ { { std::forward<Args>(args)... } } {};

        const T& Peek() const { return buffer_[currentIndex_]; }
        T& Peek() { return buffer_[currentIndex_]; }

        T& Advance()
        {
            currentIndex_++;

            if (currentIndex_ == buffer_.size())
                currentIndex_ = 0;

            return buffer_[currentIndex_];
        }

        std::array<T, SIZE>& GetBuffer() { return buffer_; }

    private:
        std::array<T, SIZE> buffer_;
        size_t currentIndex_ = 0;
    };
}