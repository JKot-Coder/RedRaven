#pragma once

#include "Resource.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR::GAPI
{
    enum class ShaderStage : uint8_t
    {
        Vertex,
        Pixel,
        Compute,
        Geometry,
        Hull,
        Domain,
        Amplification,
        Mesh,
        RayGen,
        RayMiss,
        RayClosestHit,
        RayAnyHit,
        RayIntersection,
        Callable,
        Tile, // METAL only
        Count
    };

    enum class ShaderStageMask : uint32_t
    {
        None = 0,
        Vertex = 1 << (uint32_t)ShaderStage::Vertex,
        Pixel = 1 << (uint32_t)ShaderStage::Pixel,
        Compute = 1 << (uint32_t)ShaderStage::Compute,
        Geometry = 1 << (uint32_t)ShaderStage::Geometry,
        Hull = 1 << (uint32_t)ShaderStage::Hull,
        Domain = 1 << (uint32_t)ShaderStage::Domain,
        Amplification = 1 << (uint32_t)ShaderStage::Amplification,
        Mesh = 1 << (uint32_t)ShaderStage::Mesh,
        RayGen = 1 << (uint32_t)ShaderStage::RayGen,
        RayMiss = 1 << (uint32_t)ShaderStage::RayMiss,
        RayClosestHit = 1 << (uint32_t)ShaderStage::RayClosestHit,
        RayAnyHit = 1 << (uint32_t)ShaderStage::RayAnyHit,
        RayIntersection = 1 << (uint32_t)ShaderStage::RayIntersection,
        Callable = 1 << (uint32_t)ShaderStage::Callable,
        Tile = 1 << (uint32_t)ShaderStage::Tile, // METAL only
        All = (1 << (uint32_t)ShaderStage::Count) - 1,
    };
    ENUM_CLASS_BITWISE_OPS(ShaderStageMask);

    constexpr ShaderStageMask GetShaderStageMask(ShaderStage stage)
    {
        ASSERT(stage < ShaderStage::Count);
        return static_cast<ShaderStageMask>(1 << eastl::to_underlying(stage));
    }

    struct ShaderDesc
    {
    public:
        ShaderDesc() = default;

        ShaderDesc(ShaderStage stage, const std::byte* data, size_t size)
            : stage(stage), data(data), size(size) { }

    public:
        ShaderStage stage;
        const std::byte* data;
        size_t size;
    };

    class IShader
    {
    public:
        virtual ~IShader() = default;
    };

    class Shader final : public Resource<IShader>
    {
    public:
        const ShaderDesc& GetDesc() const { return desc_; }

    private:
        Shader(const ShaderDesc& desc, const std::string& name)
            : Resource(Type::Shader, name), desc_(desc) { }

    private:
        friend class Render::DeviceContext;

        ShaderDesc desc_;
    };
}