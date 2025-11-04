#pragma once

#include <fmt/core.h>

#define THROW(...) throw std::runtime_error(fmt::format(__VA_ARGS__));
#define UNREACHABLE() throw std::runtime_error("Unreachable");

namespace RR
{
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
     * ENUM_INFO(Foo, {
     *     { Foo::A, "A" },
     *     { Foo::B, "B" },
     *     { Foo::C, "C" },
     * })
     */
#define ENUM_INFO(T, ...)                                             \
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
     * ENUM_REGISTER(Bar::Foo)
     * } // namespace ns
     *
     * Registered enums can be converted to/from strings using:
     * - enumToString<Enum>(Enum value)
     * - stringToEnum<Enum>(std::string_view name)
     */
    #define ENUM_REGISTER(T)                                            \
    constexpr T##_info findEnumInfoADL [[maybe_unused]] (T) noexcept \
    {                                                                      \
        return T##_info{};                                                 \
    }

}