#pragma once

// Wyhash by default
#include "common/hash/Wyhash.hpp"

namespace RR::Common::Hash::Default
{
    using HashType = uint64_t;

    constexpr inline HashType Hash(const std::string& str) { return RR::Common::Hash::Wyhash::Hash<64>(str.c_str(), (uint32_t)str.size()); }
    constexpr inline HashType Hash(const void* data, std::size_t len) { return RR::Common::Hash::Wyhash::Hash<64>(data, (uint32_t)len); }

    constexpr inline HashType operator"" _h(const char* str, std::size_t len)
    {
        return RR::Common::Hash::Wyhash::ForceConstexprHash<64>(str, (uint32_t)len);
    }
}