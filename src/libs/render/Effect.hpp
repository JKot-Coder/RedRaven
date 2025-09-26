#pragma once

#include "gapi/Limits.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "absl/container/flat_hash_map.h"

#include "common/hashing/Wyhash.hpp"

namespace RR::Render
{
    using PsoHasher = RR::Common::Wyhash::WyHash<64>;
    using PsoHashType = PsoHasher::HashType;

    struct GraphicsParams
	{
        GAPI::PrimitiveTopology primitiveTopology;
        const GAPI::VertexLayout* vertexLayout = nullptr;
        uint32_t renderTargetCount = 0;
        eastl::array<GAPI::GpuResourceFormat, GAPI::MAX_RENDER_TARGETS_COUNT> renderTargetFormats;
        GAPI::GpuResourceFormat depthStencilFormat;

        void Reset()
        {
            primitiveTopology = GAPI::PrimitiveTopology::TriangleList;
            renderTargetCount = 0;
            vertexLayout = nullptr;
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
        // todo trivial hash
        absl::flat_hash_map<PsoHashType, eastl::unique_ptr<GAPI::PipelineState>> pipelineStates;
    };
}