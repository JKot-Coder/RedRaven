#pragma once

#include "Utils.hpp"

enum class ShaderResourceType
{
    TextureSrv,
    TextureUav,
    RawBufferSrv,
    RawBufferUav,
    TypedBufferSrv,
    TypedBufferUav,
    Cbv,
    StructuredBufferUav,
    StructuredBufferSrv,
    AccelerationStructureSrv,
    Dsv,
    Rtv,
    Sampler,

    Count
};

#if FALCOR_HAS_D3D12
enum class ShaderVisibility
{
    None = 0,
    Vertex = (1 << (uint32_t)ShaderType::Vertex),
    Pixel = (1 << (uint32_t)ShaderType::Pixel),
    Hull = (1 << (uint32_t)ShaderType::Hull),
    Domain = (1 << (uint32_t)ShaderType::Domain),
    Geometry = (1 << (uint32_t)ShaderType::Geometry),
    Compute = (1 << (uint32_t)ShaderType::Compute),

    All = (1 << (uint32_t)ShaderType::Count) - 1,
};

FALCOR_ENUM_CLASS_OPERATORS(ShaderVisibility);


class D3D12DescriptorSetLayout
{
public:
    struct Range
    {
        ShaderResourceType type;
        uint32_t baseRegIndex;
        uint32_t descCount;
        uint32_t regSpace;
    };

    D3D12DescriptorSetLayout(ShaderVisibility visibility = ShaderVisibility::All) : mVisibility(visibility) {}
    D3D12DescriptorSetLayout& addRange(ShaderResourceType type, uint32_t baseRegIndex, uint32_t descriptorCount, uint32_t regSpace = 0)
    {
        Range r;
        r.descCount = descriptorCount;
        r.baseRegIndex = baseRegIndex;
        r.regSpace = regSpace;
        r.type = type;

        mRanges.push_back(r);
        return *this;
    }
    size_t getRangeCount() const { return mRanges.size(); }
    const Range& getRange(size_t index) const { return mRanges[index]; }
    ShaderVisibility getVisibility() const { return mVisibility; }

private:
    std::vector<Range> mRanges;
    ShaderVisibility mVisibility;
};
#endif