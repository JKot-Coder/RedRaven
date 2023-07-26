#pragma once

namespace RR
{
    namespace Rfx
    {
        struct UnownedStringSlice
        {
        public:
            UnownedStringSlice() = default;

            UnownedStringSlice(const U8String& string)
                : begin_(string.c_str()), end_(string.c_str() + string.length())
            {
            }

            UnownedStringSlice(const U8Char* begin, const U8Char* end)
                : begin_(begin), end_(end)
            {
            }

            const U8Char* Begin() const { return begin_; }
            const U8Char* End() const { return end_; }
            size_t GetLength() const { return std::distance(begin_, end_); }
            U8String AsString() const { return U8String(begin_, end_); }

            bool operator==(UnownedStringSlice const& other) const;
            bool operator!=(UnownedStringSlice const& other) const
            {
                return !(*this == other);
            }

            bool operator==(char const* str) const { return (*this) == UnownedStringSlice(str); }
            bool operator!=(char const* str) const { return !(*this == str); }

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
            return std::equal(lhs.Begin(), lhs.End(), rhs.Begin(), rhs.End());
        }
    };

    template <>
    struct hash<UnownedStringSlice>
    {
        std::size_t operator()(const UnownedStringSlice& slice) const noexcept
        {
            std::size_t hash = 0;
            const RR::U8Char* ptr = slice.Begin();
            const RR::U8Char* end = slice.End();
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
        return formatter<string_view>::format(string_view(stringSlice.Begin(), stringSlice.GetLength()), ctx);
    }
};