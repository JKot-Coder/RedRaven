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
        eastl::fixed_vector<BindingLayoutElement, MAX_BINDINGS_PER_GROUP, false> elements;
    };

    class IBindingGroupLayout
    {
    public:
        virtual ~IBindingGroupLayout() = default;
    };

    class BindingGroupLayout final : public Resource<IBindingGroupLayout, true>
    {
    public:
        BindingGroupLayout(const BindingGroupLayoutDesc& desc, const std::string& name)
            : Resource(Type::BindingGroupLayout, name), desc_(desc)
        {
        }

        const BindingGroupLayoutDesc& GetDesc() const { return desc_; }

    private:
        BindingGroupLayoutDesc desc_;
    };


}

