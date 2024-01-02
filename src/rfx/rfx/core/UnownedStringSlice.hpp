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
            UnownedStringSlice head(size_t idx) const
            {
                ASSERT(idx >= 0 && idx <= length());
                return UnownedStringSlice(begin_, idx);
            }
            // Return a tail of the slice - everything from the index to the end of the slice
            UnownedStringSlice tail(size_t idx) const
            {
                ASSERT(idx >= 0 && idx <= length());
                return UnownedStringSlice(begin_ + idx, end_);
            }

            size_t length() const { return std::distance(begin_, end_); }
            U8String AsString() const { return U8String(begin_, end_); }

            bool operator==(UnownedStringSlice const& other) const;
            bool operator!=(UnownedStringSlice const& other) const
            {
                return !(*this == other);
            }

            bool operator==(char const* str) const { return (*this) == UnownedStringSlice(str); }
            bool operator!=(char const* str) const { return !(*this == str); }

            bool isStartsWith(UnownedStringSlice const& other) const
            {
                const auto thisSize = length();
                const auto otherSize = other.length();

                if (otherSize > thisSize)
                    return false;

                return head(otherSize) == other;
            }

            bool isStartsWith(char const* string) const
            {
                ASSERT(string != nullptr);
                return isStartsWith(UnownedStringSlice(string));
            }

            void reset()
            {
                begin_ = nullptr;
                end_ = nullptr;
            }

        private:
            const U8Char* begin_;
            const U8Char* end_;
        };

        template <U8Char Delimiter>
        struct StringSplitIterator
        {
            StringSplitIterator(const U8Char* parentBegin, const U8Char* parentEnd) : begin_(parentBegin),
                                                                                      parentEnd_(parentEnd)
            {
                ASSERT(parentBegin && parentEnd);
                ASSERT(parentBegin <= parentEnd);
                end_ = std::find(begin_, parentEnd_, Delimiter);
            }

            StringSplitIterator& operator*() { return *this; }
            const StringSplitIterator& operator*() const { return this; }

            StringSplitIterator<Delimiter>& operator++()
            {
                begin_ = end_ + (end_ < parentEnd_);
                end_ = std::find(begin_, parentEnd_, Delimiter);
                return *this;
            }

            bool operator==(const StringSplitIterator<Delimiter>& other) const
            {
                return begin_ == other.begin_ &&
                       end_ == other.end_ &&
                       parentEnd_ == other.parentEnd_;
            }

            bool operator!=(const StringSplitIterator<Delimiter>& other) const { return !(*this == other); }

            UnownedStringSlice slice() const { return UnownedStringSlice(begin_, end_); }

        private:
            const U8Char* begin_;
            const U8Char* end_;
            const U8Char* parentEnd_;
        };

        template <typename T>
        struct IsStringView
        {
            using TWithoutConst = std::remove_const_t<T>;
            static constexpr bool value = std::is_same_v<decltype(std::declval<TWithoutConst>().begin()), U8Char*> &&
                                          std::is_same_v<decltype(std::declval<TWithoutConst>().end()), U8Char*>;
        };

        template <typename T>
        struct StringSplit
        {
            typedef StringSplitIterator<'.'> iterator;
            typedef const StringSplitIterator<'.'> const_iterator;

            StringSplit(T stringView) : stringView_(stringView) { }

            iterator begin() { return iterator(stringView_.begin(), stringView_.end()); }
            const_iterator begin() const { return const_iterator(stringView_.begin(), stringView_.end()); }
            iterator end() { return iterator(stringView_.end(), stringView_.end()); }
            const_iterator end() const { return const_iterator(stringView_.end(), stringView_.end()); }

            size_t length() const { return stringView_.length(); }

        private:
            T stringView_;
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
        return formatter<string_view>::format(string_view(stringSlice.begin(), stringSlice.length()), ctx);
    }
};

template <typename T>
struct fmt::formatter<RR::Rfx::StringSplit<T>> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::StringSplit<T> stringSplit, FormatContext& ctx)
    {
        return formatter<string_view>::format(string_view(stringSplit.begin().slice().begin(), stringSplit.length()), ctx);
    }
};