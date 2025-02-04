#pragma once

namespace RR
{
    namespace ParseTools
    {
        struct UnownedStringSlice
        {
        public:
            typedef char* iterator;
            typedef const char* const_iterator;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

            UnownedStringSlice() = default;
            UnownedStringSlice(const char* string) : begin_(string), end_(string + strlen(string)) { }
            UnownedStringSlice(const std::string& string) : begin_(string.c_str()), end_(string.c_str() + string.length()) { }
            UnownedStringSlice(const_iterator begin, const_iterator end) : begin_(begin), end_(end) { }
            UnownedStringSlice(const_iterator begin, size_t len) : begin_(begin), end_(begin + len) { }

            const_iterator data() const { return begin_; }

            const_iterator begin() const { return begin_; }
            const_iterator end() const { return end_; }
            const_reverse_iterator rbegin() const { return const_reverse_iterator(end_); }
            const_reverse_iterator rend() const { return const_reverse_iterator(begin_); }

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

            bool empty() const { return length() == 0; }
            size_t size() const { return std::distance(begin_, end_); }
            size_t length() const { return std::distance(begin_, end_); }
            std::string asString() const { return std::string(begin_, end_); }

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
            const_iterator begin_;
            const_iterator end_;
        };

        template <char Delimiter>
        struct StringSplitIterator
        {
            StringSplitIterator(const char* parentBegin, const char* parentEnd) : begin_(parentBegin),
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
            const char* begin_;
            const char* end_;
            const char* parentEnd_;
        };

        template <typename T, char Delimiter>
        struct StringSplit
        {
            typedef StringSplitIterator<Delimiter> iterator;
            typedef const StringSplitIterator<Delimiter> const_iterator;

            StringSplit(T stringView) : stringView_(stringView) { }

            iterator begin() { return iterator(&*stringView_.begin(), &*stringView_.end()); }
            const_iterator begin() const { return const_iterator(&*stringView_.begin(), &*stringView_.end()); }
            iterator end() { return iterator(&*stringView_.end(), &*stringView_.end()); }
            const_iterator end() const { return const_iterator(&*stringView_.end(), &*stringView_.end()); }

            size_t length() const { return stringView_.length(); }

        private:
            T stringView_;
        };
    }
}

template <>
struct fmt::formatter<RR::ParseTools::UnownedStringSlice> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::ParseTools::UnownedStringSlice stringSlice, FormatContext& ctx)
    {
        return formatter<string_view>::format(string_view(stringSlice.begin(), stringSlice.length()), ctx);
    }
};

template <typename T, char Delimiter>
struct fmt::formatter<RR::ParseTools::StringSplit<T, Delimiter>> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::ParseTools::StringSplit<T, Delimiter> stringSplit, FormatContext& ctx)
    {
        return formatter<string_view>::format(string_view(stringSplit.begin().slice().begin(), stringSplit.length()), ctx);
    }
};