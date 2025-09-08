#pragma once

#include <string>
#include "EASTL/string.h"
#include "common/hashing/HashType.hpp"
// Wyhash by default
#include "common/hashing/Wyhash.hpp"

namespace RR::Common::Hashing::Default
{
    static constexpr size_t HashBits = 32;
    using HashType = typename HashType_t<HashBits>::type;

    constexpr HashType Hash(const void* data, std::size_t len)
    {
        ASSERT_MSG(data, "Hash: data pointer cannot be null");
        return Wyhash::Hash<HashBits>(data, static_cast<uint32_t>(len));
    }

    constexpr HashType Hash(const char* str)
    {
        return Wyhash::Hash<HashBits>(str, std::char_traits<char>::length(str));
    }

    template <typename T>
    constexpr HashType Hash(const T& value)
    {
        return Wyhash::Hash<HashBits>(&value, sizeof(T));
    }

    template <>
    constexpr HashType Hash<std::string>(const std::string& value)
    {
        return Wyhash::Hash<HashBits>(
            value.data(),
            value.size());
    }

    template <>
    constexpr HashType Hash<eastl::string>(const eastl::string& value)
    {
        return Wyhash::Hash<HashBits>(
            value.data(),
            value.size()
        );
    }

    constexpr HashType operator""_h(const char* str, std::size_t len)
    {
        return Wyhash::ForceConstexprHash<HashBits>(str, static_cast<uint32_t>(len));
    }

    template <size_t Bits = HashBits, typename T>
    constexpr void HashCombine(typename HashType_t<Bits>::type& hash, T value) noexcept
    {
        if constexpr (Bits == 32)
        {
            hash ^= Hash(value) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
        }
        else if constexpr (Bits == 64)
        {
            hash ^= Hash(value) + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
        }
        else
        {
            static_assert(false, "Bits must be 32 or 64");
        }
    }
}