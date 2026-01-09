#pragma once

#include "effect_library/EffectLibrary.hpp"
#include "effect_library/EffectFormat.hpp"

#include "common/ChunkAllocator.hpp"
#include "absl/container/flat_hash_map.h"

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    class EffectSerializer
    {
    public:
        EffectSerializer(): stringAllocator(1024) { };
        ~EffectSerializer() = default;

        uint32_t AddString(const std::string_view& str);
        uint32_t AddShader(const EffectLibrary::ShaderDesc& shader);
        uint32_t AddEffect(const EffectLibrary::EffectDesc& effect);

        Common::RResult Serialize(const std::string& path);

    private:
        Common::ChunkAllocator stringAllocator;
        absl::flat_hash_map<std::string, uint32_t> stringsCache;
        std::vector<std::byte> shaderData;
        std::vector<std::byte> effectsData;
        uint32_t stringsCount = 0;
        uint32_t effectsCount = 0;
        uint32_t shadersCount = 0;
    };
}