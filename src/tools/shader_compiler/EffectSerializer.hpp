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

    struct UniformDesc
    {
        std::string name;
        EffectLibrary::Asset::FieldType type;

        uint32_t arraySize;
        uint32_t offset;
        uint32_t size;
        uint32_t layoutIndex = EffectLibrary::Asset::INVALID_INDEX;
    };

    struct ResourceReflection
    {
        enum class Type : int32_t
        {
            Texture,
            StructuredBuffer,
            RawBuffer,
            TypedBuffer,
            Sampler,
            ConstantBuffer,
            AccelerationStructure,
        };

        enum class Access : int32_t
        {
            Read,
            Write,
            ReadWrite
        };

        std::string name;
        Type type;
        Access access;

        GAPI::GpuResourceDimension dimension;
        uint32_t bindingIndex;
        GAPI::TextureSampleType sampleType = GAPI::TextureSampleType::Undefined;
        GAPI::GpuResourceFormat format = GAPI::GpuResourceFormat::Unknown;
        uint32_t count;
        uint32_t layoutIndex = EffectLibrary::Asset::INVALID_INDEX;

        GAPI::ShaderStageMask usageMask = GAPI::ShaderStageMask::None;
    };

    struct BindGroupDesc
    {
        std::string name;
        uint32_t bindingSpace;
        uint32_t uniformCBV;
        uint32_t resourcesLayoutIndex;
        uint32_t childsLayoutIndex;
    };

    struct PassDesc
    {
        std::string name;
        GAPI::RasterizerDesc rasterizerDesc;
        GAPI::DepthStencilDesc depthStencilDesc;
        GAPI::BlendDesc blendDesc;
        uint32_t rootBindingGroupIndex;
        eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count)> shaderIndexes;
    };

    struct EffectDesc
    {
        std::string name;
        eastl::vector<PassDesc> passes;
    };

    class EffectSerializer
    {
    public:
        EffectSerializer();
        ~EffectSerializer() = default;

        uint32_t AddString(const std::string_view& str);
        uint32_t AddShader(const EffectLibrary::ShaderDesc& shader);
        uint32_t AddUniform(const UniformDesc& uniform);
        uint32_t AddLayout(const eastl::span<uint32_t>& layout);
        uint32_t AddResource(const ResourceReflection& resource);
        uint32_t AddEffect(const EffectDesc& effect);
        uint32_t AddBindGroup(const BindGroupDesc& bindGroup);

        Common::RResult Serialize(const std::string& path);

    private:
        Common::ChunkAllocator stringAllocator;
        absl::flat_hash_map<std::string, uint32_t> stringsCache;

        std::vector<std::byte> shadersData;
        std::vector<std::byte> effectsData;
        std::vector<std::byte> srvData;
        std::vector<std::byte> uavData;
        std::vector<std::byte> cbvData;
        std::vector<std::byte> uniformsData;
        std::vector<std::byte> layoutsData;
        std::vector<std::byte> bindGroupsData;
        uint32_t stringsCount = 0;
        uint32_t effectsCount = 0;
        uint32_t shadersCount = 0;
        uint32_t srvCount = 0;
        uint32_t uavCount = 0;
        uint32_t cbvCount = 0;
        uint32_t uniformsCount = 0;
        uint32_t layoutsCount = 0;
        uint32_t bindGroupsCount = 0;
    };
}