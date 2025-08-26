#include "Effect.hpp"

#include "common/hash/Common.hpp"

#include "render/DeviceContext.hpp"
#include "gapi/PipelineState.hpp"

namespace RR::Render
{
    Effect::~Effect(){};

    Effect::Effect(const std::string& name) { UNUSED(name); }

    void combineGraphicsParamsHash(PsoHashType& psoHash, const GraphicsParams& params)
    {
        static_assert(sizeof(GraphicsParams) == 44);

        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.renderTargetCount);
        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.primitiveTopology);
        for(size_t i = 0; i < params.renderTargetCount; ++i)
            Common::Hash::HashCombine<PsoHashBits>(psoHash, params.renderTargetFormats[i]);

        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.depthStencilFormat);
    }

    GAPI::GraphicPipelineState* Effect::EvaluateGraphicsPipelineState(const GraphicsParams& params)
    {
        PsoHashType psoHash = 0xcbf29ce484222325;
        combineGraphicsParamsHash(psoHash, params);

        auto it = pipelineStates.find(psoHash);

        if(it != pipelineStates.end())
        {
            ASSERT(dynamic_cast<GAPI::GraphicPipelineState*>(it->second.get()));
            return static_cast<GAPI::GraphicPipelineState*>(it->second.get());
        }
        GAPI::GraphicPipelineStateDesc graphicPSODesc;
        graphicPSODesc.primitiveTopology = params.primitiveTopology;

        ASSERT(params.renderTargetCount <= graphicPSODesc.renderTargetFormats.size());
        ASSERT(params.renderTargetCount <= params.renderTargetFormats.size());

        graphicPSODesc.renderTargetCount = params.renderTargetCount;

        for(size_t i = 0; i < params.renderTargetCount; ++i)
            graphicPSODesc.renderTargetFormats[i] = params.renderTargetFormats[i];

        graphicPSODesc.depthStencilFormat = params.depthStencilFormat;

        const auto& deviceContext = DeviceContext::Instance();
        GAPI::GraphicPipelineState::UniquePtr pipelineState = deviceContext.CreatePipelineState(graphicPSODesc, "Effect");

        auto pso = pipelineStates.emplace(psoHash, std::move(pipelineState)).first->second.get();
        return static_cast<GAPI::GraphicPipelineState*>(pso);
    }
}