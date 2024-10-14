
#pragma once

#include "common/hash/Wyhash.hpp"
#include "EASTL/fixed_string.h"

namespace RR::Ecs
{
    using HashType = uint64_t;

    template<typename stringType>
    constexpr inline HashType Hash(const stringType& string) {return RR::Common::Hash::Wyhash::Hash<64>(string.data(), (uint32_t)string.size()); }
    constexpr inline HashType Hash(const char* cstr) { return RR::Common::Hash::Wyhash::Hash<64>(cstr, (uint32_t)std::char_traits<char>::length(cstr)); }
    constexpr inline HashType Hash(const void* data, std::size_t len) { return RR::Common::Hash::Wyhash::Hash<64>(data, (uint32_t)len); }

    constexpr inline HashType operator"" _h(const char* str, std::size_t len)
    {
        return RR::Common::Hash::Wyhash::ForceConstexprHash<64>(str, (uint32_t)len);
    }

    template<int FixedSize>
    struct HashString
    {
        template <typename stringType>
        constexpr inline HashString(const stringType& str) : string(str.data(), str.size()), hash(Hash(str)) { }
        constexpr inline HashString(const char* cstr) : string(cstr), hash(Hash(cstr)) { }
        constexpr inline HashString() = default;

        eastl::fixed_string<char, FixedSize> string;
        HashType hash;
    };
}