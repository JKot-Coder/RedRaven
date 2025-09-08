#pragma once

#include "Resource.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR::GAPI
{
    enum class ShaderType : uint8_t
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

    struct ShaderDesc
    {
    public:
        ShaderDesc() = default;

        ShaderDesc(ShaderType type, const std::byte* data, size_t size)
            : type(type), data(data), size(size) { }

    public:
        ShaderType type;
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
        using UniquePtr = eastl::unique_ptr<Shader>;

        const ShaderDesc& GetDesc() const { return desc_; }

    private:
        static UniquePtr Create(const ShaderDesc& desc, const std::string& name)
        {
            return UniquePtr(new Shader(desc, name));
        }

        Shader(const ShaderDesc& desc, const std::string& name)
            : Resource(Type::Shader, name), desc_(desc) { }

    private:
        friend class Render::DeviceContext;

        ShaderDesc desc_;
    };
}