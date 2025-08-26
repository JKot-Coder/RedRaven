#include "Effect.hpp"

#include "common/hash/Common.hpp"

#include "render/DeviceContext.hpp"

namespace RR::Render
{
    Effect::~Effect(){};

    Effect::Effect(const std::string& name) { UNUSED(name); }

    void combineGraphicsParamsHash(PsoHashType& psoHash, const EvaluateGraphicsParams& params)
    {
        static_assert(sizeof(EvaluateGraphicsParams) == 44);

        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.renderTargetCount);
        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.primitiveTopology);
        for(size_t i = 0; i < params.renderTargetCount; ++i)
            Common::Hash::HashCombine<PsoHashBits>(psoHash, params.renderTargetFormats[i]);

        Common::Hash::HashCombine<PsoHashBits>(psoHash, params.depthStencilFormat);
    }

    GAPI::PipelineState* Effect::EvaluateGraphicsPipelineState(const EvaluateGraphicsParams& params)
    {
        PsoHashType psoHash = 0xcbf29ce484222325;
        combineGraphicsParamsHash(psoHash, params);

        auto it = pipelineStates.find(psoHash);

        if(it != pipelineStates.end())
            return it->second.get();

        GAPI::GraphicPipelineStateDesc pipelineStateDesc;
        pipelineStateDesc.primitiveTopology = params.primitiveTopology;

        ASSERT(params.renderTargetCount <= pipelineStateDesc.renderTargetFormats.size());
        ASSERT(params.renderTargetCount <= params.renderTargetFormats.size());

        for(size_t i = 0; i < params.renderTargetCount; ++i)
            pipelineStateDesc.renderTargetFormats[i] = params.renderTargetFormats[i];

        pipelineStateDesc.depthStencilFormat = params.depthStencilFormat;

        const auto& deviceContext = DeviceContext::Instance();
        auto pipelineState = deviceContext.CreatePipelineState(pipelineStateDesc, "Effect");

        return pipelineStates.emplace(psoHash, std::move(pipelineState)).first->second.get();
    }
}