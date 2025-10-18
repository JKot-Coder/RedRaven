#include "ShaderImpl.hpp"

#include "RenderDevice.h"
#include "Shader.h"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    ShaderImpl::~ShaderImpl() { }

    DL::SHADER_TYPE getShaderType(GAPI::ShaderType shaderType)
    {
        switch (shaderType)
        {
            case GAPI::ShaderType::Vertex: return DL::SHADER_TYPE_VERTEX;
            case GAPI::ShaderType::Pixel: return DL::SHADER_TYPE_PIXEL;
            case GAPI::ShaderType::Compute: return DL::SHADER_TYPE_COMPUTE;
            case GAPI::ShaderType::Geometry: return DL::SHADER_TYPE_GEOMETRY;
            case GAPI::ShaderType::Hull: return DL::SHADER_TYPE_HULL;
            case GAPI::ShaderType::Domain: return DL::SHADER_TYPE_DOMAIN;
            case GAPI::ShaderType::Amplification: return DL::SHADER_TYPE_AMPLIFICATION;
            case GAPI::ShaderType::Mesh: return DL::SHADER_TYPE_MESH;
            case GAPI::ShaderType::RayGen: return DL::SHADER_TYPE_RAY_GEN;
            case GAPI::ShaderType::RayMiss: return DL::SHADER_TYPE_RAY_MISS;
            case GAPI::ShaderType::RayClosestHit: return DL::SHADER_TYPE_RAY_CLOSEST_HIT;
            case GAPI::ShaderType::RayAnyHit: return DL::SHADER_TYPE_RAY_ANY_HIT;
            case GAPI::ShaderType::RayIntersection: return DL::SHADER_TYPE_RAY_INTERSECTION;
            case GAPI::ShaderType::Callable: return DL::SHADER_TYPE_CALLABLE;
            case GAPI::ShaderType::Tile: return DL::SHADER_TYPE_TILE;
            default:
                ASSERT_MSG(false, "Unknown shader type");
                return DL::SHADER_TYPE_UNKNOWN;
        }
    }

    DL::ShaderCreateInfo getShaderCreateInfo(const GAPI::ShaderDesc& desc, const std::string& name)
    {
        DL::ShaderCreateInfo shaderCI;
        shaderCI.ByteCode = desc.data;
        shaderCI.ByteCodeSize = desc.size;
        shaderCI.Desc.Name = name.c_str();
        shaderCI.Desc.ShaderType = getShaderType(desc.type);
        shaderCI.LoadConstantBufferReflection = false;
        return shaderCI;
    }

    void ShaderImpl::Init(DL::IRenderDevice* device, Shader& resource)
    {
        DL::ShaderCreateInfo shaderCI = getShaderCreateInfo(resource.GetDesc(), resource.GetName());

        DL::IShader* shaderPtr = nullptr;
        device->CreateShader(shaderCI, &shaderPtr, nullptr);
        shader.Attach(shaderPtr);
    }
}
