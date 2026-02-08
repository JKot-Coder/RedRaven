#pragma once

#include "gapi/Limits.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"
#include "gapi/RenderPassDesc.hpp"

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


        void SetRenderPass(const GAPI::RenderPassDesc& renderPass)
        {
            colorAttachmentCount = renderPass.colorAttachmentCount;

            for (size_t i = 0; i < renderPass.colorAttachmentCount; ++i)
            {
                const auto& colorAttachment = renderPass.colorAttachments[i];
                const auto* renderTargetView = colorAttachment.renderTargetView;
                colorAttachmentFormats[i] = renderTargetView ? renderTargetView->GetDesc().format : GAPI::GpuResourceFormat::Unknown;
            }

            const auto* depthStencilView = renderPass.depthStencilAttachment.depthStencilView;
            depthStencilFormat = depthStencilView ? depthStencilView->GetDesc().format : GAPI::GpuResourceFormat::Unknown;

            dirty = true;
        }

        void SetVertexLayout(const GAPI::VertexLayout* vertexLayout)
        {
            this->vertexLayout = vertexLayout;
            dirty = true;
        }

        void Reset()
        {
            dirty = true;
            primitiveTopology = GAPI::PrimitiveTopology::TriangleList;
            colorAttachmentCount = 0;
            vertexLayout = nullptr;
            colorAttachmentFormats.fill(GAPI::GpuResourceFormat::Unknown);
            depthStencilFormat = GAPI::GpuResourceFormat::Unknown;
        }

        HashType GetHash() const
        {
            if (dirty)
            {
                static_assert(sizeof(GraphicsParams) == 64);

                HashBuilder<PsoHasher> hashBuilder;
                hashBuilder.Combine(colorAttachmentCount);
                hashBuilder.Combine(primitiveTopology);
                if (vertexLayout)
                    hashBuilder.Combine(vertexLayout->GetHash());
                for (size_t i = 0; i < colorAttachmentCount; ++i)
                    hashBuilder.Combine(colorAttachmentFormats[i]);
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
        uint32_t colorAttachmentCount = 0;
        eastl::array<GAPI::GpuResourceFormat, GAPI::MAX_COLOR_ATTACHMENT_COUNT> colorAttachmentFormats;
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
        ~Effect();

        GAPI::GraphicPipelineState* EvaluateGraphicsPipelineState(const GraphicsParams& params);

    private:
        friend class DeviceContext;

        Effect(const std::string& name, EffectDesc&& effectDesc);

    private:
        EffectDesc effectDesc;
        // todo trivial hash
        absl::flat_hash_map<PsoHashType, eastl::unique_ptr<GAPI::GraphicPipelineState>> pipelineStates;
    };
}