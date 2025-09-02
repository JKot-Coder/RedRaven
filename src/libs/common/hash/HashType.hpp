#pragma once

#include <cstdint>

namespace RR::Common::Hashing
{
    template <size_t HashBits>
    struct HashType_t;

    template <>
    struct HashType_t<8>
    {
        using type = uint8_t;
    };

    template <>
    struct HashType_t<16>
    {
        using type = uint16_t;
    };

    template <>
    struct HashType_t<32>
    {
        using type = uint32_t;
    };

    template <>
    struct HashType_t<64>
    {
        using type = uint64_t;
    };
}