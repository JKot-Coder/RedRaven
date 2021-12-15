#pragma once

#define ENUM_CLASS_OPERATORS(EnumType)      \
    ENUM_CLASS_BINARY_OPERATOR(EnumType, &) \
    ENUM_CLASS_BINARY_OPERATOR(EnumType, |) \
    ENUM_CLASS_UNARY_OPERATOR(EnumType, ~)

#define ENUM_CLASS_FRIEND_OPERATORS(EnumType)      \
    ENUM_CLASS_FRIEND_BINARY_OPERATOR(EnumType, &) \
    ENUM_CLASS_FRIEND_BINARY_OPERATOR(EnumType, |) \
    ENUM_CLASS_FRIEND_UNARY_OPERATOR(EnumType, ~)

#define ENUM_CLASS_FRIEND_BINARY_OPERATOR(EnumType, Op) ENUM_CLASS_BINARY_OPERATOR(EnumType, Op, friend)
#define ENUM_CLASS_FRIEND_UNARY_OPERATOR(EnumType, Op) ENUM_CLASS_UNARY_OPERATOR(EnumType, Op, friend)

#define ENUM_CLASS_BINARY_OPERATOR(EnumType, Op, Friend)                                                          \
    constexpr Friend inline EnumType operator Op(EnumType lhs, EnumType rhs)                                      \
    {                                                                                                             \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                              \
        static_assert(std::is_enum<EnumType>::value, "EnumType shoud be be an enum.");                            \
        static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");         \
        return static_cast<EnumType>(static_cast<UnderlyingType>(lhs) Op static_cast<UnderlyingType>(rhs));       \
    }                                                                                                             \
    inline EnumType Friend operator Op##=(EnumType& lhs, EnumType rhs)                                            \
    {                                                                                                             \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                              \
        return lhs = static_cast<EnumType>(static_cast<UnderlyingType>(lhs) Op static_cast<UnderlyingType>(rhs)); \
    }

#define ENUM_CLASS_UNARY_OPERATOR(EnumType, Op, Friend)                                                   \
    constexpr Friend inline EnumType operator Op(EnumType val)                                            \
    {                                                                                                     \
        using UnderlyingType = std::underlying_type<EnumType>::type;                                      \
        static_assert(std::is_enum<EnumType>::value, "EnumType shoud be be an enum.");                    \
        static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected."); \
        return static_cast<EnumType>(Op static_cast<UnderlyingType>(val));                                \
    }

namespace RR
{
    namespace Common
    {
        struct EnumClassHash
        {
            template <typename T>
            std::size_t operator()(T value) const noexcept
            {
                using uderlyingType = std::underlying_type<T>;

                static_assert(std::is_enum<T>::value, "Must be a scoped enum!");
                static_assert(!std::is_convertible<T, typename uderlyingType::type>::value, "Must be a scoped enum!");

                return std::hash<typename uderlyingType::type> {}(static_cast<typename uderlyingType::type>(value));
            }
        };

        template <typename EnumT>
        bool IsSet(EnumT value, EnumT flag)
        {
            using UnderlyingType = std::underlying_type<EnumT>;
            static_assert(std::is_enum<EnumT>::value, "EnumType shoud be be an enum.");
            static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");
            return ((static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flag)) == static_cast<UnderlyingType>(flag));
        }

        template <typename EnumT>
        bool IsAny(EnumT value, EnumT flag)
        {
            using UnderlyingType = std::underlying_type<EnumT>;
            static_assert(std::is_enum<EnumT>::value, "EnumType shoud be be an enum.");
            static_assert(std::is_unsigned<UnderlyingType>::value, "Unsigned underlying type are expected.");
            return ((static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flag)) != static_cast<UnderlyingType>(0));
        }
    }
}