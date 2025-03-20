#pragma once

namespace RR::Ecs
{
    /**
     * @brief A strongly-typed index template class
     *
     * @tparam Tag Type tag used to differentiate different kinds of indices
     * @tparam IndexType Underlying type used to store the index value
     */
    template <typename Tag, typename IndexType>
    struct Index final
    {
    public:
        using ThisType = Index<Tag, IndexType>;
        using ValueType = IndexType;
        static constexpr ValueType InvalidValue = static_cast<IndexType>(-1);
        static constexpr Index Invalid() { return Index{}; };

        constexpr Index() noexcept = default;
        explicit constexpr Index(ValueType value) noexcept : value(value) { }

        static constexpr Index FromValue(ValueType value) noexcept { return Index(value); }

        constexpr IndexType GetRaw() const noexcept { return value; } // Todo naming. GetValue in EntityId vs value.
        [[nodiscard]] constexpr bool IsValid() const noexcept { return value != InvalidValue; }

        constexpr bool operator==(const ThisType& other) const noexcept { return value == other.value; }
        constexpr bool operator!=(const ThisType& other) const noexcept { return value != other.value; }
        constexpr bool operator<(const ThisType& other) const noexcept { return value < other.value; }
        constexpr bool operator<=(const ThisType& other) const noexcept { return value <= other.value; }
        constexpr bool operator>(const ThisType& other) const noexcept { return value > other.value; }
        constexpr bool operator>=(const ThisType& other) const noexcept { return value >= other.value; }
        explicit operator bool() const { return value != InvalidValue; }

    private:
        IndexType value = InvalidValue;
    };
}

namespace eastl
{
    template <typename Tag, typename IndexType>
    struct hash<RR::Ecs::Index<Tag, IndexType>>
    {
        constexpr size_t operator()(const RR::Ecs::Index<Tag, IndexType>& index) const noexcept
        {
            return static_cast<size_t>(index.GetRaw());
        }
    };
}