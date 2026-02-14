#include "EffectSerializer.hpp"

#include "common/Result.hpp"

#include <fstream>

#define THROW(message)                     \
    {                                      \
        ASSERT_MSG(false, message);        \
        throw std::runtime_error(message); \
    }                                      \
    (void)0

namespace RR
{
    using namespace EffectLibrary;

    std::string getShaderStageName(GAPI::ShaderStage stage)
    {
        switch (stage)
        {
            case GAPI::ShaderStage::Vertex: return "Vertex";
            case GAPI::ShaderStage::Pixel: return "Pixel";
            case GAPI::ShaderStage::Compute: return "Compute";
            case GAPI::ShaderStage::Geometry: return "Geometry";
            case GAPI::ShaderStage::Hull: return "Hull";
            case GAPI::ShaderStage::Domain: return "Domain";
            case GAPI::ShaderStage::Amplification: return "Amplification";
            case GAPI::ShaderStage::Mesh: return "Mesh";
            case GAPI::ShaderStage::RayGen: return "RayGen";
            case GAPI::ShaderStage::RayMiss: return "RayMiss";
            case GAPI::ShaderStage::RayClosestHit: return "RayClosestHit";
            case GAPI::ShaderStage::RayAnyHit: return "RayAnyHit";
            case GAPI::ShaderStage::RayIntersection: return "RayIntersection";
            case GAPI::ShaderStage::Callable: return "Callable";
            case GAPI::ShaderStage::Tile: return "Tile";
            default: return "Unknown";
        }
    }

    std::string getUsageMaskString(GAPI::ShaderStageMask usageMask)
    {
        std::string usageMaskString;
        for (uint32_t i = 0; i < eastl::to_underlying(GAPI::ShaderStage::Count); i++)
        {
            if (Common::IsSet(usageMask, GAPI::GetShaderStageMask(static_cast<GAPI::ShaderStage>(i))))
                usageMaskString += getShaderStageName(static_cast<GAPI::ShaderStage>(i)) + " ";
        }
        return usageMaskString;
    }

    void insertData(std::vector<std::byte>& data, const void* ptr, size_t size)
    {
        data.reserve(data.size() + size);
        data.insert(data.end(), reinterpret_cast<const std::byte*>(ptr), reinterpret_cast<const std::byte*>(ptr) + size);
    }

    template <typename T>
    void insertData(std::vector<std::byte>& data, const T& value)
    {
        insertData(data, reinterpret_cast<const void*>(&value), sizeof(value));
    }

    uint32_t EffectSerializer::AddString(const std::string_view& str)
    {
        auto it = stringsCache.find(str);
        if (it != stringsCache.end())
            return it->second;

        stringAllocator.allocateString(str);
        stringsCache[str] = stringsCount;
        return stringsCount++;
    }

    uint32_t EffectSerializer::AddShader(const ShaderDesc& shader)
    {
        shadersData.reserve(shadersData.size() + shader.size + sizeof(Asset::ShaderDesc));

        Asset::ShaderDesc shaderDesc;
        shaderDesc.nameIndex = AddString(shader.name);
        shaderDesc.stage = shader.stage;
        shaderDesc.size = static_cast<uint32_t>(shader.size);

        insertData(shadersData, shaderDesc);
        insertData(shadersData, shader.data, shader.size);

        return shadersCount++;
    }

    uint32_t EffectSerializer::AddLayout(const eastl::span<uint32_t>& layout)
    {
        Asset::Layout layoutAsset;
        layoutAsset.elementsCount = static_cast<uint32_t>(layout.size());
        insertData(layoutsData, layoutAsset);
        insertData(layoutsData, layout.data(), layout.size() * sizeof(uint32_t));
        return layoutsCount++;
    }

    uint32_t EffectSerializer::AddResource(const RR::ResourceReflection& resource)
    {
        std::cout << "resource: " << resource.name
                  << " binding index:" << resource.bindingIndex
                  // << " dimension: " << enumToString(resource.dimension)
                  << " usage mask: " << getUsageMaskString(resource.usageMask)
                  //   << " type: " << enumToString(resource.type)
                  << std::endl;

        switch (resource.type)
        {
            case RR::ResourceReflection::Type::Texture:
            case RR::ResourceReflection::Type::StructuredBuffer:
            case RR::ResourceReflection::Type::RawBuffer:
            case RR::ResourceReflection::Type::TypedBuffer:
            {
                if (resource.access == RR::ResourceReflection::Access::Read)
                {
                    Asset::SrvReflection srvDesc;
                    srvDesc.nameIndex = AddString(resource.name);
                    srvDesc.usageMask = resource.usageMask;
                    srvDesc.dimension = resource.dimension;
                    srvDesc.sampleType = resource.sampleType;
                    srvDesc.binding = resource.bindingIndex;
                    srvDesc.count = resource.count;

                    insertData(srvData, srvDesc);

                    auto index = srvCount++;
                    //currentLayout.resources.push_back(index);
                    return index;
                }
                else
                {
                    Asset::UavReflection uavDesc;
                    uavDesc.nameIndex = AddString(resource.name);
                    uavDesc.usageMask = resource.usageMask;
                    uavDesc.dimension = resource.dimension;
                    uavDesc.format = resource.format;
                    uavDesc.binding = resource.bindingIndex;
                    uavDesc.count = resource.count;

                    insertData(uavData, uavDesc);

                    auto index = uavCount++;
                    //currentLayout.resources.push_back(index);
                    return index;
                }
            }
            case RR::ResourceReflection::Type::ConstantBuffer:
            {
                Asset::CbvReflection cbvDesc;
                cbvDesc.nameIndex = AddString(resource.name);
                cbvDesc.usageMask = resource.usageMask;
                cbvDesc.binding = resource.bindingIndex;
                cbvDesc.count = resource.count;
                cbvDesc.layoutIndex = resource.layoutIndex;

                insertData(cbvData, cbvDesc);

                auto index = cbvCount++;
                return index;
            }

            default:
                THROW("Invalid type");
        }
        /*
                switch (resource.type)
                {
                    case ResourceReflection::Type::Texture:
                        break;
                    case ResourceReflection::Type::StructuredBuffer:
                        break;
                    case ResourceReflection::Type::RawBuffer:
                        break;
                }
                Asset::SrvReflection srvDesc;
                srvDesc.nameIndex = AddString(srv.name);
                srvDesc.stageMask = srv.stageMask;
                srvDesc.dimension = srv.dimension;
                srvDesc.sampleType = srv.sampleType;
                srvDesc.binding = srv.binding;
                srvDesc.set = srv.set;
                srvDesc.count = srv.count;

                insertData(srvData, srv);

                return srvCount++;*/
        UNUSED(resource);
        return 0;
    }

    uint32_t EffectSerializer::AddUniform(const UniformDesc& uniform)
    {
        UNUSED(uniform);

        std::cout << "add uniform: " << uniform.name << std::endl;
        return uniformsCount++;
    }

    uint32_t EffectSerializer::AddEffect(const RR::EffectDesc& effect)
    {
        Asset::EffectDesc effectDesc;
        effectDesc.nameIndex = AddString(effect.name);
        effectDesc.passCount = static_cast<uint32_t>(effect.passes.size());
        insertData(effectsData, effectDesc);

        for (auto& pass : effect.passes)
        {
            Asset::PassDesc passDesc;
            passDesc.nameIndex = AddString(pass.name);
            passDesc.rasterizerDesc = pass.rasterizerDesc;
            passDesc.depthStencilDesc = pass.depthStencilDesc;
            passDesc.blendDesc = pass.blendDesc;
            passDesc.shaderStages = GAPI::ShaderStageMask::None;

            eastl::fixed_vector<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count), false> shaderIndexes;
            for (uint32_t i = 0; i < pass.shaderIndexes.size(); i++)
            {
                if (pass.shaderIndexes[i] == Asset::INVALID_INDEX)
                    continue;

                passDesc.shaderStages |= GetShaderStageMask(static_cast<GAPI::ShaderStage>(i));
                shaderIndexes.push_back(pass.shaderIndexes[i]);
            }

            insertData(effectsData, passDesc);
            insertData(effectsData, shaderIndexes.data(), shaderIndexes.size() * sizeof(uint32_t));
        }

        return effectsCount++;
    }

    uint32_t EffectSerializer::AddBindGroup(const BindGroupDesc& bindGroup)
    {
        Asset::BindGroup bindGroupAsset;
        bindGroupAsset.nameIndex = AddString(bindGroup.name);
        bindGroupAsset.bindingSpace = bindGroup.bindingSpace;
        bindGroupAsset.uniformCBV = bindGroup.uniformCBV;
        bindGroupAsset.resourcesLayoutIndex = bindGroup.resourcesLayoutIndex;

        insertData(bindGroupsData, bindGroupAsset);
        return bindGroupsCount++;
    }

    Common::RResult EffectSerializer::Serialize(const std::string& path)
    {
      //  if (!stack.empty())
     //  // {
      //      std::cerr << "Bind group is not closed, failed to serialize" << std::endl;
      //      return Common::RResult::Fail;
      //  }

        std::cout << "Save shader library: " << path << std::endl;

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return Common::RResult::Fail;
        }

        uint32_t stringsSectionSize = 0;
        for (auto& chunk : stringAllocator)
            stringsSectionSize += static_cast<uint32_t>(chunk.allocated);

        Asset::Header header;
        header.magic = Asset::Header::MAGIC;
        header.version = Asset::Header::VERSION;
        header.stringsSectionSize = stringsSectionSize;
        header.stringsCount = stringsCount;
        header.shadersSectionSize = shadersData.size();
        header.shadersCount = shadersCount;
        header.srvSectionSize = srvData.size();
        header.srvCount = srvCount;
        header.uavSectionSize = uavData.size();
        header.uavCount = uavCount;
        header.cbvSectionSize = cbvData.size();
        header.cbvCount = cbvCount;
        header.effectsSectionSize = effectsData.size();
        header.effectsCount = effectsCount;
        header.uniformsSectionSize = uniformsData.size();
        header.uniformsCount = uniformsCount;
        header.layoutsSectionSize = layoutsData.size();
        header.layoutsCount = layoutsCount;
        header.bindGroupsSectionSize = bindGroupsData.size();
        header.bindGroupsCount = bindGroupsCount;

        // Header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Strings
        for (auto& chunk : stringAllocator)
            file.write(reinterpret_cast<const char*>(chunk.buffer.get()), chunk.allocated);

        // Shaders
        file.write(reinterpret_cast<const char*>(shadersData.data()), shadersData.size());

        // SRV UAV CBV
        file.write(reinterpret_cast<const char*>(srvData.data()), srvData.size());
        file.write(reinterpret_cast<const char*>(uavData.data()), uavData.size());
        file.write(reinterpret_cast<const char*>(cbvData.data()), cbvData.size());

        // Layouts
        file.write(reinterpret_cast<const char*>(layoutsData.data()), layoutsData.size());

        // Binding groups
        file.write(reinterpret_cast<const char*>(bindGroupsData.data()), bindGroupsData.size());

        // Effects
        file.write(reinterpret_cast<const char*>(effectsData.data()), effectsData.size());

        if (file.fail())
        {
            std::cerr << "Failed to write to file: " << path << std::endl;
            return Common::RResult::Fail;
        }

        file.close();
        return Common::RResult::Ok;
    }
}