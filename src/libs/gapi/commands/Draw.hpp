#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/Resource.hpp"
#include "gapi/PipelineState.hpp"

namespace RR::GAPI::Commands
{
    struct Draw : public Command
    {
        struct Attribs
        {
            uint32_t vertexCount = 0;
            uint32_t startVertex = 0;
            uint32_t instanceCount = 0;
        };

        Draw(const Attribs& attribs, GraphicPipelineState* pso) : Command(Type::Draw), attribs(attribs)
        {
            ASSERT(pso);
            psoImpl = pso->GetPrivateImpl<GAPI::IPipelineState>();
            ASSERT(psoImpl);
        }

        Attribs attribs;
        IPipelineState* psoImpl = nullptr;
    };
}