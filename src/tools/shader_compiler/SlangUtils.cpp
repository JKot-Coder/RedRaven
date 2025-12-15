#include "SlangUtils.hpp"

#include "slang.h"
#include "gapi/Shader.hpp"

namespace RR
{
    GAPI::ShaderStage GetShaderStage(SlangStage stage)
    {
        switch (stage)
        {
            case SLANG_STAGE_VERTEX: return GAPI::ShaderStage::Vertex;
            case SLANG_STAGE_HULL: return GAPI::ShaderStage::Hull;
            case SLANG_STAGE_DOMAIN: return GAPI::ShaderStage::Domain;
            case SLANG_STAGE_GEOMETRY: return GAPI::ShaderStage::Geometry;
            case SLANG_STAGE_FRAGMENT: return GAPI::ShaderStage::Pixel;
            case SLANG_STAGE_COMPUTE: return GAPI::ShaderStage::Compute;
            case SLANG_STAGE_RAY_GENERATION: return GAPI::ShaderStage::RayGen;
            case SLANG_STAGE_INTERSECTION: return GAPI::ShaderStage::RayIntersection;
            case SLANG_STAGE_ANY_HIT: return GAPI::ShaderStage::RayAnyHit;
            case SLANG_STAGE_CLOSEST_HIT: return GAPI::ShaderStage::RayClosestHit;
            case SLANG_STAGE_MISS: return GAPI::ShaderStage::RayMiss;
            case SLANG_STAGE_CALLABLE: return GAPI::ShaderStage::Callable;
            case SLANG_STAGE_MESH: return GAPI::ShaderStage::Mesh;
            case SLANG_STAGE_AMPLIFICATION: return GAPI::ShaderStage::Amplification;
            default: throw std::runtime_error("Unknown shader stage");
        }
    }
}