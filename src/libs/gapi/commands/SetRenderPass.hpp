#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/RenderPassDesc.hpp"

namespace RR::GAPI::Commands
{
    struct BeginRenderPass : public Command
    {
        BeginRenderPass(const RenderPassDesc& desc) : Command(Type::BeginRenderPass), desc(desc) { }

        RenderPassDesc desc;
    };

    struct EndRenderPass : public Command
    {
        EndRenderPass() : Command(Type::EndRenderPass) { }
    };
}