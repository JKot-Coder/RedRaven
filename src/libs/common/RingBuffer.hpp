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

        RingBuffer() = default;

        template <typename... Args>
        RingBuffer(Args&&... args) : buffer_ {{std::forward<Args>(args)...}} {};

        size_t size() const { return buffer_.size(); }
        size_t capacity() const { return buffer_.capacity(); }
        auto begin() { return buffer_.begin(); }
        auto end()   { return buffer_.end(); }

        const T& Peek() const { return buffer_[currentIndex_]; }
        T& Peek() { return buffer_[currentIndex_]; }

        T& Advance()
        {
            currentIndex_ = (++currentIndex_) % buffer_.size();
            return buffer_[currentIndex_];
        }

    private:
        std::array<T, SIZE> buffer_;
        size_t currentIndex_ = 0;
    };
}