#include "BindingGroupLayout.hpp"

namespace RR::GAPI
{
    void BindingGroupLayout::InitReflection(const BindingGroupReflectionDesc& desc)
    {
        bindingSpace      = desc.bindingSpace;
        uniformBufferSize = desc.uniformBufferSize;
        uniformCbvSlot    = desc.uniformCbvSlot;

        for (const auto& f : desc.fields)
        {
            const uint32_t index = static_cast<uint32_t>(fields.size());
            fields.push_back(f);
            fieldMap[f.nameHash] = index;
        }

        for (const auto& r : desc.resources)
        {
            const uint32_t index = static_cast<uint32_t>(resourceSlots.size());
            resourceSlots.push_back(r);
            resourceMap[r.nameHash] = index;
        }
    }

    uint32_t BindingGroupLayout::FindFieldIndex(Common::HashType nameHash) const
    {
        const auto it = fieldMap.find(nameHash);
        return it != fieldMap.end() ? it->second : INVALID_SLOT;
    }

    uint32_t BindingGroupLayout::FindResourceSlotIndex(Common::HashType nameHash) const
    {
        const auto it = resourceMap.find(nameHash);
        return it != resourceMap.end() ? it->second : INVALID_SLOT;
    }
}
