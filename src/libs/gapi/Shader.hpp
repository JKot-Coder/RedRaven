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
    };

    struct ShaderDescription
    {
    public:
        ShaderDescription() = default;

        ShaderDescription(ShaderType shaderType, const std::string& entryPoint, const std::string& source)
            : shaderType(shaderType), entryPoint(entryPoint), source(source) { }

    public:
        ShaderType shaderType;
        std::string entryPoint;
        std::string source;
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

        const ShaderDescription& GetDescription() const { return description_; }

    private:
        static UniquePtr Create(const ShaderDescription& description, const std::string& name)
        {
            return UniquePtr(new Shader(description, name));
        }

        Shader(const ShaderDescription& description, const std::string& name)
            : Resource(Type::Shader, name), description_(description) { }

    private:
        friend class Render::DeviceContext;

        ShaderDescription description_;
    };
}