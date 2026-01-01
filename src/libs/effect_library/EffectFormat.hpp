#pragma once

#include "gapi/Shader.hpp"
#include "gapi/PipelineState.hpp"

namespace RR::EffectLibrary
{
    namespace Asset
    {
        #pragma pack(push, 1)
        static constexpr uint32_t INVALID_INDEX = static_cast<uint32_t>(-1);

        struct Header
        {
            static constexpr uint32_t MAGIC = 0x4C584652;
            static constexpr uint32_t VERSION = 1;
            uint32_t magic;
            uint32_t version;
            uint32_t stringSectionSize;
            uint32_t stringsCount;
            uint32_t shadersSectionSize;
            uint32_t shadersCount;
            uint32_t effectsSectionSize;
            uint32_t effectsCount;
        };

        struct ShaderDesc
        {
            struct Header
            {
                uint32_t nameIndex;
                GAPI::ShaderStage stage;
                uint32_t size;
            } header;
            std::vector<std::byte> data;
        };

        struct PassDesc
        {
            uint32_t nameIndex;
            GAPI::BlendDesc blendDesc;
            GAPI::RasterizerDesc rasterizerDesc;
            GAPI::DepthStencilDesc depthStencilDesc;
            eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count)> shaderIndexes;
        };

        struct EffectDesc
        {
            struct Header
            {
                uint32_t nameIndex;
                uint32_t passCount;
            } header;
            std::vector<PassDesc> passes;
        };
        #pragma pack(pop)
    }
}