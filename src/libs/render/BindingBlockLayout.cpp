#include "BindingBlockLayout.hpp"

#include "effect_library/EffectLibrary.hpp"

#include <limits>

namespace RR::Render
{
    void BindingBlockLayout::InitFromReflection(const EffectLibrary::BindingGroupReflection& group)
    {
        name = group.name;
        bindingSpace = group.bindingSpace;

        uint16_t resourceSlotIndex = 0;

        for (const auto& res : group.resources)
        {
            switch (res.type)
            {
                case GAPI::BindingType::ConstantBuffer:
                {
                    // First CBV becomes the uniform buffer slot
                    if (uniformCbvSlot == INVALID_SLOT)
                        uniformCbvSlot = res.binding;

                    for (const auto& uniform : res.uniformFields)
                    {
                        ASSERT(uniform.offset <= std::numeric_limits<uint16_t>::max());
                        ASSERT(uniform.size <= std::numeric_limits<uint16_t>::max());
                        FieldDesc field;
                        field.nameHash = Common::Hash(uniform.name);
                        field.offset = static_cast<uint16_t>(uniform.offset);
                        field.size = static_cast<uint16_t>(uniform.size);

                        const uint32_t fieldIndex = static_cast<uint32_t>(fields.size());
                        fields.push_back(field);
                        fieldMap[field.nameHash] = fieldIndex;

                        // Track maximum extent for buffer size
                        const uint32_t extent = uniform.offset + uniform.size;
                        if (extent > uniformBufferSize)
                            uniformBufferSize = extent;
                    }
                    break;
                }
                case GAPI::BindingType::TextureSRV:
                case GAPI::BindingType::TextureUAV:
                case GAPI::BindingType::BufferSRV:
                case GAPI::BindingType::BufferUAV:
                case GAPI::BindingType::Sampler:
                {
                    ResourceSlotDesc slot;
                    slot.nameHash = Common::Hash(res.name);
                    slot.binding = static_cast<uint16_t>(res.binding);
                    slot.slotIndex = resourceSlotIndex++;
                    slot.type = res.type;

                    const uint32_t index = static_cast<uint32_t>(resourceSlots.size());
                    resourceSlots.push_back(slot);
                    resourceMap[slot.nameHash] = index;
                    break;
                }
                default:
                {
                    ASSERT_MSG(false, "Unknown binding type");
                    break;
                }
            }
        }
    }

    uint32_t BindingBlockLayout::FindFieldIndex(Common::HashType nameHash) const
    {
        auto it = fieldMap.find(nameHash);
        return it != fieldMap.end() ? it->second : INVALID_SLOT;
    }

    uint32_t BindingBlockLayout::FindResourceSlotIndex(Common::HashType nameHash) const
    {
        auto it = resourceMap.find(nameHash);
        return it != resourceMap.end() ? it->second : INVALID_SLOT;
    }
}
