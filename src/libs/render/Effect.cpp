#include "Effect.hpp"

#include "render/DeviceContext.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "common/hashing/Hash.hpp"

namespace RR::Render
{
    Effect::~Effect(){};

    Effect::Effect(const std::string& name, EffectDesc&& effectDesc)
    {
        UNUSED(name);

        this->effectDesc = std::move(effectDesc);
    }


    GAPI::GraphicPipelineState* Effect::EvaluateGraphicsPipelineState(const GraphicsParams& params)
    {
        auto psoHash = params.GetHash();
        auto it = pipelineStates.find(psoHash);

        if(it != pipelineStates.end())
        {
            ASSERT(dynamic_cast<GAPI::GraphicPipelineState*>(it->second.get()));
            return static_cast<GAPI::GraphicPipelineState*>(it->second.get());
        }
        GAPI::GraphicPipelineStateDesc graphicPSODesc;
        graphicPSODesc.primitiveTopology = params.primitiveTopology;
        if(params.vertexLayout)
            graphicPSODesc.vertexLayout = *params.vertexLayout;

        ASSERT(params.renderTargetCount <= graphicPSODesc.renderTargetFormats.size());
        ASSERT(params.renderTargetCount <= params.renderTargetFormats.size());

        graphicPSODesc.renderTargetCount = params.renderTargetCount;

        // Todo maybe we need to check real shaders outputs here.
        for(size_t i = 0; i < params.renderTargetCount; ++i)
            graphicPSODesc.renderTargetFormats[i] = params.renderTargetFormats[i];

        ASSERT(effectDesc.passes.size() == 1);
        auto& pass = effectDesc.passes[0]; // TODO
        graphicPSODesc.depthStencilFormat = params.depthStencilFormat;
        graphicPSODesc.vs = pass.shaders[eastl::to_underlying(GAPI::ShaderType::Vertex)];
        graphicPSODesc.ps = pass.shaders[eastl::to_underlying(GAPI::ShaderType::Pixel)];

        const auto& deviceContext = DeviceContext::Instance();
        GAPI::GraphicPipelineState::UniquePtr pipelineState = deviceContext.CreatePipelineState(graphicPSODesc, "Effect");

        auto pso = pipelineStates.emplace(psoHash, std::move(pipelineState)).first->second.get();
        return static_cast<GAPI::GraphicPipelineState*>(pso);
    }
}