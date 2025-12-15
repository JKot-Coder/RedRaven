#include "ShaderImpl.hpp"

#include "RenderDevice.h"
#include "Shader.h"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    ShaderImpl::~ShaderImpl() { }

    DL::SHADER_TYPE getShaderType(GAPI::ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
            case GAPI::ShaderStage::Vertex: return DL::SHADER_TYPE_VERTEX;
            case GAPI::ShaderStage::Pixel: return DL::SHADER_TYPE_PIXEL;
            case GAPI::ShaderStage::Compute: return DL::SHADER_TYPE_COMPUTE;
            case GAPI::ShaderStage::Geometry: return DL::SHADER_TYPE_GEOMETRY;
            case GAPI::ShaderStage::Hull: return DL::SHADER_TYPE_HULL;
            case GAPI::ShaderStage::Domain: return DL::SHADER_TYPE_DOMAIN;
            case GAPI::ShaderStage::Amplification: return DL::SHADER_TYPE_AMPLIFICATION;
            case GAPI::ShaderStage::Mesh: return DL::SHADER_TYPE_MESH;
            case GAPI::ShaderStage::RayGen: return DL::SHADER_TYPE_RAY_GEN;
            case GAPI::ShaderStage::RayMiss: return DL::SHADER_TYPE_RAY_MISS;
            case GAPI::ShaderStage::RayClosestHit: return DL::SHADER_TYPE_RAY_CLOSEST_HIT;
            case GAPI::ShaderStage::RayAnyHit: return DL::SHADER_TYPE_RAY_ANY_HIT;
            case GAPI::ShaderStage::RayIntersection: return DL::SHADER_TYPE_RAY_INTERSECTION;
            case GAPI::ShaderStage::Callable: return DL::SHADER_TYPE_CALLABLE;
            case GAPI::ShaderStage::Tile: return DL::SHADER_TYPE_TILE;
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
        shaderCI.Desc.ShaderType = getShaderType(desc.stage);
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
