#pragma once

#include "gapi/BindingGroupLayout.hpp"
#include "gapi/Limits.hpp"

#include "common/hashing/Hash.hpp"

namespace RR::EffectLibrary
{
    struct BindingGroupReflection;
    struct ResourceReflection;
}

namespace RR::Render
{
    // Describes a single uniform field within a CBV
    struct FieldDesc
    {
        Common::HashType nameHash; // precomputed hash for fast lookup
        uint16_t offset; // byte offset in uniform buffer
        uint16_t size; // byte size of field
    };

    // Describes a single resource slot (SRV, UAV, sampler)
    struct ResourceSlotDesc
    {
        Common::HashType nameHash; // precomputed hash
        uint16_t binding; // binding index in bind group
        uint16_t slotIndex; // index in ParameterBlock::resources[]
        GAPI::BindingType type; // TextureSRV, BufferSRV, Sampler, etc.
    };

    // Immutable layout built once from EffectLibrary reflection data.
    // Shared (read-only) across all ParameterBlock instances with same layout.
    class ParameterBlockLayout
    {
    public:
        static constexpr uint32_t INVALID_SLOT = ~0u;

        ParameterBlockLayout() = default;

        void InitFromReflection(const EffectLibrary::BindingGroupReflection& group);

        const char* GetName() const { return name; }
        uint32_t GetBindingSpace() const { return bindingSpace; }
        uint32_t GetUniformBufferSize() const { return uniformBufferSize; }
        uint32_t GetUniformCbvSlot() const { return uniformCbvSlot; }

        uint32_t GetFieldCount() const { return static_cast<uint32_t>(fields.size()); }
        const FieldDesc& GetField(uint32_t index) const
        {
            ASSERT(index < fields.size());
            return fields[index];
        }

        uint32_t GetResourceSlotCount() const { return static_cast<uint32_t>(resourceSlots.size()); }
        const ResourceSlotDesc& GetResourceSlot(uint32_t index) const
        {
            ASSERT(index < resourceSlots.size());
            return resourceSlots[index];
        }

        // O(n) lookup by name hash. For hot paths, cache the index.
        uint32_t FindFieldIndex(Common::HashType nameHash) const;
        uint32_t FindResourceSlotIndex(Common::HashType nameHash) const;

    private:
        const char* name = nullptr;
        uint32_t bindingSpace = 0;
        uint32_t uniformBufferSize = 0; // total CBV size in bytes (0 if none)
        uint32_t uniformCbvSlot = INVALID_SLOT; // binding index of default CBV

        eastl::fixed_vector<FieldDesc, 16, true> fields;
        eastl::fixed_vector<ResourceSlotDesc, GAPI::MAX_BINDINGS_PER_GROUP, false> resourceSlots;
    };
}
