#pragma once

#include "magic_enum.hpp"

namespace RR::stl
{
    template <typename E>
    [[nodiscard]] constexpr auto enum_cast(magic_enum::string_view value) noexcept -> magic_enum::detail::enable_if_t<E, magic_enum::optional<std::decay_t<E>>>
    {
        return magic_enum::enum_cast<E>(value);
    }

    template <typename E>
    [[nodiscard]] constexpr auto enum_cast(magic_enum::underlying_type_t<E> value) noexcept -> magic_enum::detail::enable_if_t<E, magic_enum::optional<std::decay_t<E>>>
    {
        return magic_enum::enum_cast<E>(value);
    }
}