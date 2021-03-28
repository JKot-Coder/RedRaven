#pragma once

#define ENUM_CLASS_OPERATORS(EnumType)      \
    ENUM_CLASS_BINARY_OPERATOR(EnumType, &) \
    ENUM_CLASS_BINARY_OPERATOR(EnumType, |) \
    ENUM_CLASS_UNARY_OPERATOR(EnumType, ~)

#define ENUM_CLASS_BINARY_OPERATOR(EnumType, Op)                                                                  \
    constexpr inline EnumType operator Op(EnumType lhs, EnumType rhs)                                             \
    {                                                                                                             \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                              \
        static_assert(std::is_enum<EnumType>::value, "EnumType shoud be be an enum.");                            \
        static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");         \
        return static_cast<EnumType>(static_cast<UnderlyingType>(lhs) Op static_cast<UnderlyingType>(rhs));       \
    }                                                                                                             \
    inline EnumType operator Op##=(EnumType& lhs, EnumType rhs)                                                   \
    {                                                                                                             \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                              \
        return lhs = static_cast<EnumType>(static_cast<UnderlyingType>(lhs) Op static_cast<UnderlyingType>(rhs)); \
    }

#define ENUM_CLASS_UNARY_OPERATOR(EnumType, Op)                                                           \
    constexpr inline EnumType operator Op(EnumType val)                                                   \
    {                                                                                                     \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                      \
        static_assert(std::is_enum<EnumType>::value, "EnumType shoud be be an enum.");                    \
        static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected."); \
        return static_cast<EnumType>(Op static_cast<UnderlyingType>(val));                                \
    }

namespace OpenDemo
{
    namespace Common
    {
        struct EnumClassHash
        {
            template <typename T>
            std::size_t operator()(T t) const noexcept
            {
                return std::hash<std::underlying_type<T>>(t);
            }
        };

        template <typename EnumT>
        bool IsSet(EnumT value, EnumT flag)
        {
            using UnderlyingType = std::underlying_type<EnumT>::type;
            static_assert(std::is_enum<EnumT>::value, "EnumType shoud be be an enum.");
            static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");
            return ((static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flag)) == static_cast<UnderlyingType>(flag));
        }

        template <typename EnumT>
        bool IsAny(EnumT value, EnumT flag)
        {
            using UnderlyingType = std::underlying_type<EnumT>::type;
            static_assert(std::is_enum<EnumT>::value, "EnumType shoud be be an enum.");
            static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");
            return ((static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flag)) != static_cast<UnderlyingType>(0));
        }
    }
}