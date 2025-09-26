#pragma once

#include "common/hashing/Wyhash.hpp"

namespace RR::Common
{
    using DefaultHasher = Wyhash::WyHash<32>;
    using HashType = typename DefaultHasher::HashType;

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType Hash(const void* data, size_t len)
    {
        return Hasher::Hash(data, len);
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType Hash(const char* str)
    {
        return Hasher::Hash(str, std::char_traits<char>::length(str));
    }

    template <typename Hasher = DefaultHasher, typename T>
    static constexpr typename Hasher::HashType Hash(const T& value)
    {
        return Hasher::Hash(&value, sizeof(T));
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType Hash(const std::string& value)
    {
        return Hasher::Hash(value.data(), value.size());
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType get_hash_slot_fn(const eastl::string& value)
    {
        return Hasher::Hash(value.data(), value.size());
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType ConstexprHash(const char* str, size_t len)
    {
        return Hasher::ConstexprHash(str, len);
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType ConstexprHash(const char* str)
    {
        return Hasher::ConstexprHash(str, std::char_traits<char>::length(str));
    }

    template <typename Hasher = DefaultHasher, typename T>
    static constexpr typename Hasher::HashType ConstexprHash(const T& value)
    {
        return Hasher::ConstexprHash(&value, sizeof(T));
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType ConstexprHash(const std::string& value)
    {
        return Hasher::ConstexprHash(value.data(), value.size());
    }

    template <typename Hasher = DefaultHasher>
    static constexpr typename Hasher::HashType ConstexprHash(const eastl::string& value)
    {
        return Hasher::ConstexprHash(value.data(), value.size());
    }

    template <typename Hasher = DefaultHasher>
    struct HashBuilder
    {
        using HashType = typename Hasher::HashType;

        HashBuilder& Combine(const void* data, size_t len)
        {
            if constexpr (std::is_same_v<HashType, uint32_t>)
            {
                hash ^= Hasher::Hash(data, len) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
            }
            else if constexpr (std::is_same_v<HashType, uint64_t>)
            {
                hash ^= Hasher::Hash(data, len) + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
            }
            else
            {
                static_assert(false, "Unsupported hash type");
            }

            return *this;
        }

        HashBuilder& Combine(const char* str) { return Combine(str, std::char_traits<char>::length(str)); }
        template <typename T>
        HashBuilder& Combine(const T& value) { return Combine(&value, sizeof(T)); }
        HashBuilder& Combine(const std::string& value) { return Combine(value.data(), value.size()); }
        HashBuilder& Combine(const eastl::string& value) { return Combine(value.data(), value.size()); }

        [[nodiscard]] HashType GetHash() const { return hash; }

        static constexpr HashType DefaultHash = std::is_same_v<HashType, uint32_t> ? 0x9e3779b9u : 0x9e3779b97f4a7c15ull;

        HashType hash = DefaultHash;
    };

    constexpr HashType operator""_h(const char* str, std::size_t len)
    {
        return ConstexprHash<DefaultHasher>(str, len);
    }
}