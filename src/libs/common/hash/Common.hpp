#pragma once

#include <string>
#include "EASTL/string.h"
// Wyhash by default
#include "common/hash/Wyhash.hpp"

namespace RR::Common::Hash
{
    using HashType = uint32_t;
    static constexpr size_t HashBits = 32;

    constexpr HashType Hash(const void* data, std::size_t len)
    {
        ASSERT_MSG(data, "Hash: data pointer cannot be null");
        return RR::Common::Hash::Wyhash::Hash<HashBits>(data, static_cast<uint32_t>(len));
    }

    template <typename T>
    constexpr HashType Hash(const T& value)
    {
        return RR::Common::Hash::Wyhash::Hash<HashBits>(&value, sizeof(T));
    }

    template <>
    constexpr HashType Hash<std::string>(const std::string& value)
    {
        return RR::Common::Hash::Wyhash::Hash<HashBits>(
            value.data(),
            value.size());
    }

    template <>
    constexpr HashType Hash<eastl::string>(const eastl::string& value)
    {
        return RR::Common::Hash::Wyhash::Hash<HashBits>(
            value.data(),
            value.size()
        );
    }

    constexpr HashType operator""_h(const char* str, std::size_t len)
    {
        return RR::Common::Hash::Wyhash::ForceConstexprHash<HashBits>(str, static_cast<uint32_t>(len));
    }

    namespace detail
    {
        template <size_t Bits>
        struct hash_combine_impl;

        template <>
        struct hash_combine_impl<32>
        {
            template <typename T>
            static constexpr void hash_combine(uint32_t& seed, T value) noexcept
            {
                seed ^= Hash(value) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
            }
        };

        template <>
        struct hash_combine_impl<64>
        {
            template <typename T>
            static constexpr void hash_combine(uint64_t& seed, T value) noexcept
            {
                seed ^= Hash(value) + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
            }
        };
    }

    template <typename T>
    constexpr void HashCombine(HashType& seed, T value) noexcept
    {
        detail::hash_combine_impl<HashBits>::hash_combine(seed, value);
    }
}