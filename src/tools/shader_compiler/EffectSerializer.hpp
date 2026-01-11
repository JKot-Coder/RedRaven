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

    struct SrvReflectionDesc
    {
        std::string name;
        GAPI::ShaderStageMask stageMask;
        GAPI::GpuResourceDimension dimension;
        GAPI::TextureSampleType sampleType;
        uint32_t binding;
        uint32_t set;
        uint32_t count;
    };

    class EffectSerializer
    {
    public:
        EffectSerializer(): stringAllocator(1024) { };
        ~EffectSerializer() = default;

        uint32_t AddString(const std::string_view& str);
        uint32_t AddShader(const EffectLibrary::ShaderDesc& shader);
        uint32_t AddSrv(const SrvReflectionDesc& srv);
        uint32_t AddEffect(const EffectLibrary::EffectDesc& effect);

        Common::RResult Serialize(const std::string& path);

    private:
        Common::ChunkAllocator stringAllocator;
        absl::flat_hash_map<std::string, uint32_t> stringsCache;
        std::vector<std::byte> shaderData;
        std::vector<std::byte> effectsData;
        std::vector<std::byte> srvData;
        std::vector<std::byte> uavData;
        std::vector<std::byte> cbvData;
        uint32_t stringsCount = 0;
        uint32_t effectsCount = 0;
        uint32_t shadersCount = 0;
        uint32_t srvCount = 0;
        uint32_t uavCount = 0;
        uint32_t cbvCount = 0;
    };
}