#pragma once

#include "gapi/Shader.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/BindingGroupLayout.hpp"

#include <vector>
#include <cstdint>
#include <cstddef>

namespace RR::EffectLibrary
{
    namespace Asset
    {
        #pragma pack(push, 1)
        static constexpr uint32_t INVALID_INDEX = static_cast<uint32_t>(-1);

        enum class FieldType : uint8_t
        {
            Array,
            Struct,

            Bool,
            Bool2,
            Bool3,
            Bool4,

            Uint8,
            Uint8_2,
            Uint8_3,
            Uint8_4,

            Uint16,
            Uint16_2,
            Uint16_3,
            Uint16_4,

            Uint,
            Uint2,
            Uint3,
            Uint4,

            Uint64,
            Uint64_2,
            Uint64_3,
            Uint64_4,

            Int8,
            Int8_2,
            Int8_3,
            Int8_4,

            Int16,
            Int16_2,
            Int16_3,
            Int16_4,

            Int,
            Int2,
            Int3,
            Int4,

            Int64,
            Int64_2,
            Int64_3,
            Int64_4,

            Float16,
            Float16_2,
            Float16_3,
            Float16_4,

            Float16_2x2,
            Float16_2x3,
            Float16_2x4,
            Float16_3x2,
            Float16_3x3,
            Float16_3x4,
            Float16_4x2,
            Float16_4x3,
            Float16_4x4,

            Float,
            Float2,
            Float3,
            Float4,

            Float2x2,
            Float2x3,
            Float2x4,
            Float3x2,
            Float3x3,
            Float3x4,
            Float4x2,
            Float4x3,
            Float4x4,

            Float64,
            Float64_2,
            Float64_3,
            Float64_4,
        };

        struct SrvReflection
        {
            uint32_t nameIndex;
            GAPI::ShaderStageMask usageMask;
            GAPI::GpuResourceDimension dimension;
            GAPI::TextureSampleType sampleType;

            uint32_t binding; // Slot index
            uint32_t count; // 0 or 1 = not array, >1 = array
        };

        struct UavReflection
        {
            uint32_t nameIndex;
            GAPI::ShaderStageMask usageMask;
            GAPI::GpuResourceDimension dimension;
            GAPI::GpuResourceFormat format;
            // TODO: Store acess type.

            uint32_t binding; // Slot indexx
            uint32_t count; // 0 or 1 = not array, >1 = array
        };

        struct CbvReflection
        {
            uint32_t nameIndex;
            GAPI::ShaderStageMask usageMask;

            uint32_t binding; // Slot index
            uint32_t count; // 0 or 1 = not array, >1 = array

            uint32_t layoutIndex;
        };

        struct UniformReflection
        {
            uint32_t nameIndex;
            FieldType type;

            uint32_t offset; // Offset in bytes relative to the start of the parent structure/buffer
            uint32_t size; // Size in bytes

            uint32_t arraySize; // 0 or 1 = not array, >1 = array
            uint32_t layoutIndex; // For structs
        };

        struct Layout
        {
            uint32_t elementsCount;
            // List of indexes to fields or other resources
        };

        struct BindGroup
        {
            uint32_t nameIndex;
            uint32_t bindingSpace;
            uint32_t uniformCBV;
            uint32_t resourcesLayoutIndex;
        };

        struct Header
        {
            static constexpr uint32_t MAGIC = 0x4C584652;
            static constexpr uint32_t VERSION = 1;
            uint32_t magic;
            uint32_t version;
            uint32_t stringsSectionSize;
            uint32_t stringsCount;
            uint32_t shadersSectionSize;
            uint32_t shadersCount;
            uint32_t srvSectionSize;
            uint32_t srvCount;
            uint32_t uavSectionSize;
            uint32_t uavCount;
            uint32_t cbvSectionSize;
            uint32_t cbvCount;
            uint32_t fieldsSectionSize;
            uint32_t fieldsCount;
            uint32_t effectsSectionSize;
            uint32_t effectsCount;
            uint32_t uniformsSectionSize;
            uint32_t uniformsCount;
            uint32_t layoutsSectionSize;
            uint32_t layoutsCount;
            uint32_t bindGroupsSectionSize;
            uint32_t bindGroupsCount;
        };

        struct ShaderDesc
        {
            uint32_t nameIndex;
            GAPI::ShaderStage stage;
            uint32_t size;
            // DATA block
        };

        struct PassDesc
        {
            uint32_t nameIndex;
            GAPI::BlendDesc blendDesc;
            GAPI::RasterizerDesc rasterizerDesc;
            GAPI::DepthStencilDesc depthStencilDesc;
            GAPI::ShaderStageMask shaderStages;
            // List of shader indexes based on shaderStages mask
        };

        struct EffectDesc
        {
            uint32_t nameIndex;
            uint32_t passCount;
            // PASSES block
        };
        #pragma pack(pop)
    }
}
