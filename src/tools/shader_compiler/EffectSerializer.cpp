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
        shaderData.reserve(shaderData.size() + shader.size + sizeof(Asset::ShaderDesc::Header));

        Asset::ShaderDesc::Header header;
        header.nameIndex = AddString(shader.name);
        header.stage = shader.stage;
        header.size = static_cast<uint32_t>(shader.size);

        insertData(shaderData, header);
        insertData(shaderData, shader.data, shader.size);

        return shadersCount++;
    }

    uint32_t EffectSerializer::AddEffect(const EffectDesc& effect)
    {
        Asset::EffectDesc::Header header;
        header.nameIndex = AddString(effect.name);
        header.passCount = static_cast<uint32_t>(effect.passes.size());
        insertData(effectsData, header);

        for (auto& pass : effect.passes)
        {
            auto nameIndex = AddString(pass.name);
            insertData(effectsData, nameIndex);

            Asset::PassDesc::PSODesc psoDesc;
            psoDesc.rasterizerDesc = pass.rasterizerDesc;
            psoDesc.depthStencilDesc = pass.depthStencilDesc;
            psoDesc.blendDesc = pass.blendDesc;
            for (uint32_t i = 0; i < pass.shaderIndexes.size(); i++)
                psoDesc.shaderIndexes[i] = pass.shaderIndexes[i];
            insertData(effectsData, psoDesc);

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
                insertData(effectsData, resource);
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

        file.close();
        return Common::RResult::Ok;
    }
}