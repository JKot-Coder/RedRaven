#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/RenderPassDesc.hpp"

namespace RR::GAPI::Commands
{
    struct SetRenderPass : public Command
    {

        SetRenderPass(const RenderPassDesc& desc) : Command(Type::SetRenderPass), desc(desc) { }

        RenderPassDesc desc;
    };
}