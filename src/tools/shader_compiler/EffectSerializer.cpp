#include "EffectSerializer.hpp"

#include "common/Result.hpp"

#include <fstream>

namespace RR
{
    using namespace EffectLibrary;

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
        shaderData.reserve(shaderData.size() + shader.size + sizeof(Asset::ShaderDesc));

        Asset::ShaderDesc shaderDesc;
        shaderDesc.nameIndex = AddString(shader.name);
        shaderDesc.stage = shader.stage;
        shaderDesc.size = static_cast<uint32_t>(shader.size);

        insertData(shaderData, shaderDesc);
        insertData(shaderData, shader.data, shader.size);

        return shadersCount++;
    }

    uint32_t EffectSerializer::AddSrv(const SrvReflectionDesc& srv)
    {
        Asset::SrvReflection srvDesc;
        srvDesc.nameIndex = AddString(srv.name);
        srvDesc.stageMask = srv.stageMask;
        srvDesc.dimension = srv.dimension;
        srvDesc.sampleType = srv.sampleType;
        srvDesc.binding = srv.binding;
        srvDesc.set = srv.set;
        srvDesc.count = srv.count;

        insertData(srvData, srv);

        return srvCount++;
    }

    uint32_t EffectSerializer::AddEffect(const EffectDesc& effect)
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
                if(pass.shaderIndexes[i] == Asset::INVALID_INDEX)
                    continue;

                passDesc.shaderStages |= GetShaderStageMask(static_cast<GAPI::ShaderStage>(i));
                shaderIndexes.push_back(pass.shaderIndexes[i]);
            }

            insertData(effectsData, passDesc);
            insertData(effectsData, shaderIndexes.data(), shaderIndexes.size() * sizeof(uint32_t));
/*
            Asset::ReflectionDesc::Header reflectionHeader;
            reflectionHeader.resourcesCount = static_cast<uint32_t>(pass.reflection.resources.size());
            reflectionHeader.variablesCount = static_cast<uint32_t>(pass.reflection.fields.size());
            reflectionHeader.textureMetasCount = static_cast<uint32_t>(pass.reflection.textureMetas.size());
            reflectionHeader.rootResourceIndex = Asset::INVALID_INDEX;
            insertData(effectsData, reflectionHeader);

            for (auto& textureMeta : pass.reflection.textureMetas)
                insertData(effectsData, textureMeta);

            for (auto& field : pass.reflection.fields)
                insertData(effectsData, field);

            for (auto& resource : pass.reflection.resources)
                insertData(effectsData, resource);*/
        }

       return effectsCount++;
    }

    Common::RResult EffectSerializer::Serialize(const std::string& path)
    {
        std::cout << "Save shader library: " << path << std::endl;

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return Common::RResult::Fail;
        }

        uint32_t stringSectionSize = 0;
        for (auto& chunk : stringAllocator)
            stringSectionSize += static_cast<uint32_t>(chunk.allocated);

        Asset::Header header;
        header.magic = Asset::Header::MAGIC;
        header.version = Asset::Header::VERSION;
        header.stringSectionSize = stringSectionSize;
        header.stringsCount = stringsCount;
        header.shadersSectionSize = shaderData.size();
        header.shadersCount = shadersCount;
        header.srvSectionSize = srvData.size();
        header.srvCount = srvCount;
        header.uavSectionSize = uavData.size();
        header.uavCount = uavCount;
        header.cbvSectionSize = cbvData.size();
        header.cbvCount = cbvCount;
        header.effectsSectionSize = effectsData.size();
        header.effectsCount = effectsCount;

        // Header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Strings
        for (auto& chunk : stringAllocator)
            file.write(reinterpret_cast<const char*>(chunk.buffer.get()), chunk.allocated);

        // Shaders
        file.write(reinterpret_cast<const char*>(shaderData.data()), shaderData.size());

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