#pragma once

#include <fmt/core.h>

#define FALCOR_THROW(...) throw std::runtime_error(fmt::format(__VA_ARGS__))
#define FALCOR_UNREACHABLE() throw std::runtime_error("Unreachable")
#define UNIMPLEMENTED() assert(false); throw std::runtime_error("Unimplemented")
#define FALCOR_ASSERT(condition) ASSERT(condition)
#define FALCOR_API
#define FALCOR_OBJECT(T)

namespace
{
    struct Object
    {
    };

    template <typename T>
    using ref = std::shared_ptr<T>;

    // Helper using ADL to find EnumInfo in other namespaces.
    template <typename T>
    using EnumInfo = decltype(findEnumInfoADL(eastl::declval<T>()));

    template <typename, typename = void>
    struct has_enum_info : eastl::false_type
    {
    };

    template <typename T>
    struct has_enum_info<T, eastl::void_t<decltype(EnumInfo<T>::items)>> : eastl::true_type
    {
    };

    template <typename T>
    inline constexpr bool has_enum_info_v = has_enum_info<T>::value;

    /**
     * Convert an enum value to a string.
     * Throws if the enum value is not found in the registered enum information.
     */
    template <typename T, eastl::enable_if_t<has_enum_info_v<T>, bool> = true>
    inline const char* enumToString(T value)
    {
        const auto& items = EnumInfo<T>::items();
        auto it = eastl::find_if(eastl::begin(items), eastl::end(items), [value](const auto& item) { return item.first == value; });
        if (it == items.end())
            THROW("Invalid enum value {}", int(value));
        return it->second;
    }

    /**
     * Define enum information. This is expected to be used as follows:
     *
     * enum class Foo { A, B, C };
     * FALCOR_ENUM_INFO(Foo, {
     *     { Foo::A, "A" },
     *     { Foo::B, "B" },
     *     { Foo::C, "C" },
     * })
     */
#define FALCOR_ENUM_INFO(T, ...)                                      \
    struct T##_info                                                   \
    {                                                                 \
        static eastl::span<eastl::pair<T, const char*>> items()       \
        {                                                             \
            static eastl::pair<T, const char*> items[] = __VA_ARGS__; \
            return {eastl::begin(items), eastl::end(items)};          \
        }                                                             \
    }

    /**
     * Register enum information to be used with helper functions.
     * This needs to be placed outside of any structs but within the
     * namespace of the enum:
     *
     * namespace ns
     * {
     * struct Bar
     * {
     *     enum class Foo { A, B, C };
     *     ENUM_INFO(Foo, ...)
     * };
     *
     * FALCOR_ENUM_REGISTER(Bar::Foo)
     * } // namespace ns
     *
     * Registered enums can be converted to/from strings using:
     * - enumToString<Enum>(Enum value)
     * - stringToEnum<Enum>(std::string_view name)
     */
    #define FALCOR_ENUM_REGISTER(T)                                         \
    constexpr T##_info findEnumInfoADL [[maybe_unused]] (T) noexcept \
    {                                                                \
        return T##_info {};                                          \
    }

/**
 * Implement logical operators on a class enum for making it usable as a flags enum.
 */
// clang-format off
#define FALCOR_ENUM_CLASS_OPERATORS(e_) \
inline e_ operator& (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b)); } \
inline e_ operator| (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b)); } \
inline e_& operator|= (e_& a, e_ b) { a = a | b; return a; }; \
inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a)); } \
inline bool is_set(e_ val, e_ flag) { return (val & flag) != static_cast<e_>(0); } \
inline void flip_bit(e_& val, e_ flag) { val = is_set(val, flag) ? (val & (~flag)) : (val | flag); }
// clang-format on
}