#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/PipelineState.hpp"

namespace RR::GAPI::Commands
{
    struct DrawAttribs
    {
        uint32_t vertexCount = 0;
        uint32_t startLocation = 0;
        uint32_t instanceCount = 0;
    };

    struct VertexBinding
    {
        const IGpuResource* vertexBuffer = nullptr;
        uint32_t vertexBufferOffset = 0;
    };

    struct GeometryLayout
    {
        const IGpuResource* indexBuffer = nullptr;
        eastl::span<VertexBinding> vertexBindings;
    };

    struct Draw : public Command
    {
        Draw(const DrawAttribs& attribs, GraphicPipelineState* pso, const GeometryLayout& geometryLayout)
            : Command(Type::Draw), attribs(attribs), geometryLayout(&geometryLayout)
        {
            ASSERT(pso);
            psoImpl = pso->GetPrivateImpl<GAPI::IPipelineState>();
            ASSERT(psoImpl);
        }

        DrawAttribs attribs;
        IPipelineState* psoImpl;
        const GeometryLayout* geometryLayout;
    };

    struct DrawIndexed : public Command
    {
        DrawIndexed(const DrawAttribs& attribs, GraphicPipelineState* pso, const GeometryLayout& geometryLayout)
                : Command(Type::DrawIndexed), attribs(attribs), geometryLayout(&geometryLayout)
        {
            ASSERT(pso);
            psoImpl = pso->GetPrivateImpl<GAPI::IPipelineState>();
            ASSERT(psoImpl);
        }

        DrawAttribs attribs;
        IPipelineState* psoImpl;
        const GeometryLayout* geometryLayout;
    };
}