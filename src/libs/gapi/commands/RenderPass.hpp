#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/RenderPassDesc.hpp"

namespace RR::GAPI::Commands
{
    struct BeginRenderPass : public Command
    {
        BeginRenderPass(const RenderPassDesc& desc) : Command(Type::BeginRenderPass), desc(desc) {

            for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
            {
                ASSERT(desc.colorAttachments[i].renderTargetView);
                ASSERT(desc.colorAttachments[i].renderTargetView->GetPrivateImpl());
            }
        }

        RenderPassDesc desc;
    };

    struct EndRenderPass : public Command
    {
        EndRenderPass() : Command(Type::EndRenderPass) { }
    };
}