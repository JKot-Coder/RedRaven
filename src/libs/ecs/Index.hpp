#pragma once

namespace RR::Ecs
{
    /**
     * @brief A strongly-typed index template class
     *
     * @tparam Tag Type tag used to differentiate different kinds of indices
     * @tparam IndexType Underlying type used to store the index value
     */
    template<typename Tag, typename IndexType = uint32_t>
    class Index
    {
    public:
        using ValueType = IndexType;
        constexpr Index() noexcept : value_(static_cast<IndexType>(-1)) {}
        explicit constexpr Index(ValueType value) noexcept : value_(value) {}

        static constexpr Tag FromValue(ValueType value) noexcept {
            Tag result;
            result.value_ = value;
            return result;
        }

        constexpr IndexType Value() const noexcept { return value_; }

        constexpr bool operator==(const Index& other) const noexcept { return value_ == other.value_; }
        constexpr bool operator!=(const Index& other) const noexcept { return value_ != other.value_; }
        constexpr bool operator<(const Index& other) const noexcept { return value_ < other.value_; }
        constexpr bool operator<=(const Index& other) const noexcept { return value_ <= other.value_; }
        constexpr bool operator>(const Index& other) const noexcept { return value_ > other.value_; }
        constexpr bool operator>=(const Index& other) const noexcept { return value_ >= other.value_; }

        constexpr Index& operator++() noexcept { ++value_; return *this; }
        constexpr Index operator++(int) noexcept { Index tmp(*this); ++value_; return tmp; }
        constexpr Index& operator--() noexcept { --value_; return *this; }
        constexpr Index operator--(int) noexcept { Index tmp(*this); --value_; return tmp; }

        constexpr bool IsValid() const noexcept { return value_ != static_cast<IndexType>(-1); }

    protected:
        IndexType value_;
    };
}

namespace eastl
{
    using namespace RR::Ecs;

    template<typename Tag, typename IndexType>
    struct hash<Index<Tag, IndexType>>
    {
        size_t operator()(const Index<Tag, IndexType>& index) const noexcept
        {
            return static_cast<size_t>(index.Value());
        }
    };
}