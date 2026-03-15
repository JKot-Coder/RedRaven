#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Limits.hpp"

#include "common/hashing/Hash.hpp"

#include <EASTL/fixed_vector.h>
#include <EASTL/span.h>
#include <EASTL/vector_map.h>

namespace RR::GAPI
{

    enum class BindingType : uint8_t
    {
        ConstantBuffer,
        TextureSRV,
        TextureUAV,
        BufferSRV,
        BufferUAV,
        Sampler
    };

    struct BindingLayoutElement
    {
        BindingType          type;
        uint32_t             binding;
        uint32_t             count;
        ShaderStageMask      stageMask;
        // Populated for TextureSRV and TextureUAV
        GpuResourceDimension dimension;
        GpuResourceFormat    format;      // TextureUAV only
        TextureSampleType    sampleType;  // TextureSRV only
    };

    struct BindingGroupLayoutDesc
    {
        eastl::span<const BindingLayoutElement> elements;
    };

    // Reflection: a single uniform field within a CBV
    struct FieldDesc
    {
        Common::HashType nameHash;
        uint16_t         offset; // byte offset in uniform buffer
        uint16_t         size;   // byte size of field
    };

    // Reflection: a single resource slot (SRV, UAV, sampler)
    struct ResourceSlotDesc
    {
        Common::HashType nameHash;
        uint16_t         binding;   // binding index in bind group
        uint16_t         slotIndex; // dense index in EffectContext::resources[]
        BindingType      type;
    };

    struct BindingGroupReflectionDesc
    {
        uint32_t                            bindingSpace      = 0;
        eastl::span<const FieldDesc>        fields;
        eastl::span<const ResourceSlotDesc> resources;
        uint32_t                            uniformBufferSize = 0;
        uint32_t                            uniformCbvSlot    = ~0u;
    };

    class IBindingGroupLayout
    {
    public:
        virtual ~IBindingGroupLayout() = default;
    };

    class BindingGroupLayout final : public Resource<IBindingGroupLayout, true>
    {
    public:
        static constexpr uint32_t INVALID_SLOT = ~0u;

        BindingGroupLayout(const std::string& name)
            : Resource(Type::BindingGroupLayout, name)
        {
        }

        // Populate reflection metadata (call after GPU init).
        void InitReflection(const BindingGroupReflectionDesc& desc);

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

        // O(log n) lookup by precomputed name hash. Cache the index for hot paths.
        uint32_t FindFieldIndex(Common::HashType nameHash) const;
        uint32_t FindResourceSlotIndex(Common::HashType nameHash) const;

    private:
        using HashToIndex = eastl::pair<Common::HashType, uint32_t>;

        template <size_t N, bool Overflow = true>
        using Lookup = eastl::vector_map<
            Common::HashType, uint32_t,
            eastl::less<Common::HashType>,
            EASTLAllocatorType,
            eastl::fixed_vector<HashToIndex, N, Overflow>>;

        uint32_t bindingSpace      = 0;
        uint32_t uniformBufferSize = 0;
        uint32_t uniformCbvSlot    = INVALID_SLOT;

        eastl::fixed_vector<FieldDesc, 16, true>                             fields;
        eastl::fixed_vector<ResourceSlotDesc, MAX_BINDINGS_PER_GROUP, false> resourceSlots;

        Lookup<16>                    fieldMap;    // nameHash -> field index
        Lookup<MAX_BINDINGS_PER_GROUP, false> resourceMap; // nameHash -> slot index
    };

}
