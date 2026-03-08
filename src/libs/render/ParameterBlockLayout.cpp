#include "ParameterBlockLayout.hpp"

#include "effect_library/EffectLibrary.hpp"

#include <limits>

namespace RR::Render
{
    void ParameterBlockLayout::InitFromReflection(const EffectLibrary::BindingGroupReflection& group)
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
                        fields.push_back(field);

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

                    resourceSlots.push_back(slot);
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

    uint32_t ParameterBlockLayout::FindFieldIndex(Common::HashType nameHash) const
    {
        for (uint32_t i = 0; i < fields.size(); i++)
        {
            if (fields[i].nameHash == nameHash)
                return i;
        }

        return INVALID_SLOT;
    }

    uint32_t ParameterBlockLayout::FindResourceSlotIndex(Common::HashType nameHash) const
    {
        for (uint32_t i = 0; i < resourceSlots.size(); i++)
        {
            if (resourceSlots[i].nameHash == nameHash)
                return i;
        }

        return INVALID_SLOT;
    }
}
