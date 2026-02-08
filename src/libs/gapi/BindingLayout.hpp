#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Limits.hpp"

#include <EASTL/vector.h>

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

    struct BindingLayoutTextureMeta {
        GpuResourceDimension dimension;
        GpuResourceFormat format;
        TextureSampleType sampleType;
    };

    struct BindingLayoutElement
    {
        BindingType type;
        uint32_t    binding;
        uint32_t    count;
        ShaderStageMask stageMask;
        uint32_t textureMetaIndex;
    };

    struct BindingLayoutDesc
    {
        struct BindingGroupLayout
        {
            eastl::fixed_vector<BindingLayoutElement, MAX_BINDINGS_PER_GROUP, false> elements;
        };

        eastl::fixed_vector<BindingGroupLayout, MAX_BINDING_GROUPS, false> groups;
        eastl::fixed_vector<BindingLayoutTextureMeta, 16, false> textureMetas;
    };

    class IBindingLayout
    {
    public:
        virtual ~IBindingLayout() = default;
    };

    class BindingLayout final : public Resource<IBindingLayout, true>
    {
    public:
        using UniquePtr = eastl::unique_ptr<BindingLayout>;

        BindingLayout(const BindingLayoutDesc& desc, const std::string& name)
            : Resource(Type::BindingLayout, name), desc_(desc)
        {
        }

        const BindingLayoutDesc& GetDesc() const { return desc_; }

    private:
        BindingLayoutDesc desc_;
    };


}

