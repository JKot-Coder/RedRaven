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
    public:
        void SetPrimitiveTopology(GAPI::PrimitiveTopology topology)
        {
            primitiveTopology = topology;
            dirty = true;
        }

        void SetVertexLayout(const GAPI::VertexLayout* vertexLayout)
        {
            this->vertexLayout = vertexLayout;
            dirty = true;
        }

        void SetRenderTargetCount(uint32_t renderTargetCount)
        {
            this->renderTargetCount = renderTargetCount;
            dirty = true;
        }

        void SetRenderTargetFormat(uint32_t index, GAPI::GpuResourceFormat format)
        {
            renderTargetFormats[index] = format;
            dirty = true;
        }

        void SetDepthStencilFormat(GAPI::GpuResourceFormat format)
        {
            depthStencilFormat = format;
            dirty = true;
        }

        void Reset()
        {
            dirty = true;
            primitiveTopology = GAPI::PrimitiveTopology::TriangleList;
            renderTargetCount = 0;
            vertexLayout = nullptr;
            renderTargetFormats.fill(GAPI::GpuResourceFormat::Unknown);
            depthStencilFormat = GAPI::GpuResourceFormat::Unknown;
        }

        HashType GetHash() const
        {
            if (dirty)
            {
                static_assert(sizeof(GraphicsParams) == 64);

                HashBuilder<PsoHasher> hashBuilder;
                hashBuilder.Combine(renderTargetCount);
                hashBuilder.Combine(primitiveTopology);
                if (vertexLayout)
                    hashBuilder.Combine(vertexLayout->GetHash());
                for (size_t i = 0; i < renderTargetCount; ++i)
                    hashBuilder.Combine(renderTargetFormats[i]);
                hashBuilder.Combine(depthStencilFormat);
                hash = hashBuilder.GetHash();
                dirty = false;
            }

            return hash;
        }

    private:
        mutable bool dirty = true;

        GAPI::PrimitiveTopology primitiveTopology;
        const GAPI::VertexLayout* vertexLayout = nullptr;
        uint32_t renderTargetCount = 0;
        eastl::array<GAPI::GpuResourceFormat, GAPI::MAX_RENDER_TARGETS_COUNT> renderTargetFormats;
        GAPI::GpuResourceFormat depthStencilFormat;
        mutable PsoHashType hash;

        friend class Effect;
    };

    struct EffectDesc
    {
        struct PassDesc
        {
            const char* name;
            GAPI::RasterizerDesc rasterizerDesc;
            GAPI::DepthStencilDesc depthStencilDesc;
            GAPI::BlendDesc blendDesc;
            eastl::array<const GAPI::Shader*, eastl::to_underlying(GAPI::ShaderStage::Count)> shaders;
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