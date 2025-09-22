#pragma once

#include "gapi/Limits.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Render
{
    using PsoHashType = uint64_t;
    constexpr size_t PsoHashBits = 64;

    struct GraphicsParams
	{
        GAPI::PrimitiveTopology primitiveTopology;
        uint32_t renderTargetCount = 0;
        eastl::array<GAPI::GpuResourceFormat, GAPI::MAX_RENDER_TARGETS_COUNT> renderTargetFormats;
        GAPI::GpuResourceFormat depthStencilFormat;

        void Reset()
        {
            primitiveTopology = GAPI::PrimitiveTopology::TriangleList;
            renderTargetCount = 0;
            renderTargetFormats.fill(GAPI::GpuResourceFormat::Unknown);
            depthStencilFormat = GAPI::GpuResourceFormat::Unknown;
        }
    };

    struct EffectDesc
    {
        struct PassDesc
        {
            const char* name;
            GAPI::RasterizerDesc rasterizerDesc;
            GAPI::DepthStencilDesc depthStencilDesc;
            GAPI::BlendDesc blendDesc;
            eastl::array<const GAPI::Shader*, eastl::to_underlying(GAPI::ShaderType::Count)> shaders;
        };

        eastl::vector<PassDesc> passes;
    };

    class Effect
    {
    public:
        using UniquePtr = eastl::unique_ptr<Effect>;

        ~Effect();

        GAPI::GraphicPipelineState* EvaluateGraphicsPipelineState(const GraphicsParams& params);

    private:
        friend class DeviceContext;

        Effect(const std::string& name, EffectDesc&& effectDesc);

        static UniquePtr Create(const std::string& name, EffectDesc&& effectDesc)
        {
            return eastl::unique_ptr<Effect>(new Effect(name, std::move(effectDesc)));
        }

    private:
        EffectDesc effectDesc;
        absl::flat_hash_map<PsoHashType, eastl::unique_ptr<GAPI::PipelineState>> pipelineStates;
    };



}