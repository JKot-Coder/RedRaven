#pragma once

#include "magic_enum_containers.hpp"

namespace RR::stl
{
    template <typename V, typename E>
    using enum_array = magic_enum::containers::array<E, V>;
}