
#pragma once

#include "EASTL/fixed_string.h"
#include "common/hash/Wyhash.hpp"

namespace RR::Ecs
{
    template <int FixedSize>
    struct HashString;

    using HashType = uint32_t;
    using HashName = HashString<32>;

    namespace details
    {
        constexpr inline HashType HashImpl(const char* str, uint32_t len) { return RR::Common::Hash::Wyhash::Hash<32>(str, len); }
        constexpr inline HashType HashImpl(const void* data, uint32_t len) { return RR::Common::Hash::Wyhash::Hash<32>(data, len); }
        constexpr inline HashType ConstexprHashImpl(const char* str, uint32_t len) { return RR::Common::Hash::Wyhash::ForceConstexprHash<32>(str, len); }
    }

    template <typename stringType>
    constexpr inline HashType Hash(const stringType& string) { return details::HashImpl(string.data(), (uint32_t)string.size()); }
    constexpr inline HashType Hash(const char* cstr) { return details::HashImpl(cstr, (uint32_t)std::char_traits<char>::length(cstr)); }
    constexpr inline HashType Hash(const void* data, std::size_t len) { return details::HashImpl(data, (uint32_t)len); }
    template <typename stringType>
    constexpr inline HashType ConstexprHash(const stringType& string) { return details::ConstexprHashImpl(string.data(), (uint32_t)string.size()); }
    constexpr inline HashType ConstexprHash(const char* cstr) { return details::ConstexprHashImpl(cstr, (uint32_t)std::char_traits<char>::length(cstr)); }
    constexpr inline HashType ConstexprHash(const char* str, std::size_t len) { return details::ConstexprHashImpl(str, (uint32_t)len); }

    constexpr inline HashType operator""_h(const char* str, std::size_t len) { return details::ConstexprHashImpl(str, (uint32_t)len); }

    template <int FixedSize>
    struct HashString
    {
        template <typename stringType>
        constexpr inline HashString(const stringType& str) : string(str.data(), str.size()), hash(Hash(str)) { }
        constexpr inline HashString(const char* cstr) : string(cstr), hash(Hash(cstr)) { }
        constexpr inline HashString() = default;

        constexpr bool operator==(const HashString& other) const { return hash == other.hash; }
        constexpr operator HashType() const noexcept { return hash; }

        eastl::fixed_string<char, FixedSize> string;
        HashType hash;
    };
}