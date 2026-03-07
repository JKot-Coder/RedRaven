#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Limits.hpp"

#include <EASTL/span.h>

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

    class IBindingGroupLayout
    {
    public:
        virtual ~IBindingGroupLayout() = default;
    };

    class BindingGroupLayout final : public Resource<IBindingGroupLayout, true>
    {
    public:
        BindingGroupLayout(const std::string& name)
            : Resource(Type::BindingGroupLayout, name)
        {
        }
    };


}

