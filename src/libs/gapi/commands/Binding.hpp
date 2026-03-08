#pragma once

#include "gapi/commands/Command.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace RR::GAPI::Commands
{
    struct SetBindGroup : public Command
    {
        SetBindGroup(uint32_t group, IBindingGroup* bindGroup, const std::byte* uniformData, uint32_t uniformSize)
            : Command(Type::SetBindGroup), group(group), bindGroup(bindGroup), uniformData(uniformData), uniformSize(uniformSize)
        {
            ASSERT(bindGroup);
        }

        uint32_t group;
        IBindingGroup* bindGroup;
        const std::byte* uniformData; // Points into CommandList allocator, null if no CBV
        uint32_t uniformSize;         // 0 if no CBV in this group
    };
}
