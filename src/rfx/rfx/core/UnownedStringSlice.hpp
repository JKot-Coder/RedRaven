#pragma once

namespace RR
{
    namespace Rfx
    {
        struct UnownedStringSlice
        {
        public:
            UnownedStringSlice() = default;
            UnownedStringSlice(const U8String& string) : begin_(string.c_str()), end_(string.c_str() + string.length()) { }
            UnownedStringSlice(const U8Char* begin, const U8Char* end) : begin_(begin), end_(end) { }
            UnownedStringSlice(const U8Char* begin, size_t len) : begin_(begin), end_(begin + len) { }

            const U8Char* begin() const { return begin_; }
            const U8Char* end() const { return end_; }
            // Return a head of the slice - everything up to the index
            UnownedStringSlice Head(size_t idx) const
            {
                ASSERT(idx >= 0 && idx <= GetLength());
                return UnownedStringSlice(begin_, idx);
            }
            // Return a tail of the slice - everything from the index to the end of the slice
            UnownedStringSlice Tail(size_t idx) const
            {
                ASSERT(idx >= 0 && idx <= GetLength());
                return UnownedStringSlice(begin_ + idx, end_);
            }

            size_t GetLength() const { return std::distance(begin_, end_); }
            U8String AsString() const { return U8String(begin_, end_); }

            bool operator==(UnownedStringSlice const& other) const;
            bool operator!=(UnownedStringSlice const& other) const
            {
                return !(*this == other);
            }

            bool operator==(char const* str) const { return (*this) == UnownedStringSlice(str); }
            bool operator!=(char const* str) const { return !(*this == str); }

            bool StartsWith(UnownedStringSlice const& other) const
            {
                const auto thisSize = GetLength();
                const auto otherSize = other.GetLength();

                if (otherSize > thisSize)
                    return false;

                return Head(otherSize) == other;
            }

            bool StartsWith(char const* string) const
            {
                ASSERT(string != nullptr);
                return StartsWith(UnownedStringSlice(string));
            }

            void Reset()
            {
                begin_ = nullptr;
                end_ = nullptr;
            }

        private:
            const U8Char* begin_;
            const U8Char* end_;
        };
    }
}

namespace std
{
    using RR::Rfx::UnownedStringSlice;

    template <>
    struct equal_to<UnownedStringSlice>
    {
        bool operator()(const UnownedStringSlice& lhs, const UnownedStringSlice& rhs) const
        {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
    };

    template <>
    struct hash<UnownedStringSlice>
    {
        std::size_t operator()(const UnownedStringSlice& slice) const noexcept
        {
            std::size_t hash = 0;
            const RR::U8Char* ptr = slice.begin();
            const RR::U8Char* end = slice.end();
            while (ptr < end)
            {
                hash = static_cast<std::size_t>(*ptr) + (hash << 6) + (hash << 16) - hash;
                ptr++;
            }
            return hash;
        }
    };
}

template <>
struct fmt::formatter<RR::Rfx::UnownedStringSlice> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::UnownedStringSlice stringSlice, FormatContext& ctx)
    {
        return formatter<string_view>::format(string_view(stringSlice.begin(), stringSlice.GetLength()), ctx);
    }
};