#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/Resource.hpp"
#include "gapi/PipelineState.hpp"

namespace RR::GAPI::Commands
{
    struct DrawAttribs
    {
        uint32_t vertexCount = 0;
        uint32_t startLocation = 0;
        uint32_t instanceCount = 0;
    };

    struct Draw : public Command
    {
        Draw(const DrawAttribs& attribs, GraphicPipelineState* pso) : Command(Type::Draw), attribs(attribs)
        {
            ASSERT(pso);
            psoImpl = pso->GetPrivateImpl<GAPI::IPipelineState>();
            ASSERT(psoImpl);
        }

        DrawAttribs attribs;
        IPipelineState* psoImpl = nullptr;
    };

    struct DrawIndexed : public Command
    {
        DrawIndexed(const DrawAttribs& attribs, GraphicPipelineState* pso, const Buffer& indexBuffer) : Command(Type::DrawIndexed), attribs(attribs), indexBuffer(&indexBuffer)
        {
            ASSERT(pso);
            psoImpl = pso->GetPrivateImpl<GAPI::IPipelineState>();
            ASSERT(psoImpl);
        }

        DrawAttribs attribs;
        IPipelineState* psoImpl = nullptr;
        const Buffer* indexBuffer = nullptr;
    };
}