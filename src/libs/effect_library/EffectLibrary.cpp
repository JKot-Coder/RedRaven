#include "EffectLibrary.hpp"
#include "EffectFormat.hpp"

#include "common/io/File.hpp"
#include "common/Result.hpp"
#include "common/OnScopeExit.hpp"
#include "common/hashing/Hash.hpp"

#include "render/DeviceContext.hpp"

namespace RR::EffectLibrary
{
    Common::RResult EffectLibrary::Load(std::string_view path)
    {
        ASSERT(!loaded);
        Common::IO::File file;

        LOG_INFO("Loading effects library from file: {}", path);

        if (RR_FAILED(file.Open(path, Common::IO::FileOpenMode::Read)))
        {
            LOG_ERROR("Failed to open file: {}", path);
            return Common::RResult::Fail;
        }

        ON_SCOPE_EXIT([&file]() {
            file.Close();
        });

        Asset::Header header;
        if(file.Read(reinterpret_cast<void*>(&header), sizeof(header)) != sizeof(header))
        {
            LOG_ERROR("Failed to read header from file: {}", path);
            return Common::RResult::Fail;
        }

        if(header.magic != Asset::Header::MAGIC)
        {
            LOG_ERROR("Invalid magic number in header: {}", header.magic);
            return Common::RResult::Fail;
        }

        if(header.version != Asset::Header::VERSION)
        {
            LOG_ERROR("Invalid version in header: {}", header.version);
            return Common::RResult::Fail;
        }

        stringsData = eastl::make_unique<std::byte[]>(header.stringsSectionSize);
        if(file.Read(reinterpret_cast<void*>(stringsData.get()), header.stringsSectionSize) != header.stringsSectionSize)
        {
            LOG_ERROR("Failed to read strings data: {}", header.stringsSectionSize);
            return Common::RResult::Fail;
        }

        char* lastChar = reinterpret_cast<char*>(stringsData.get() + header.stringsSectionSize);
        char* currentChar = reinterpret_cast<char*>(stringsData.get());
        char* stringStart = currentChar;

        strings.reserve(header.stringsCount);
        while(currentChar != lastChar)
        {
            if(*currentChar == '\0')
            {
                strings.push_back(stringStart);
                stringStart = currentChar + 1;
            }

            currentChar++;
        }

        if(strings.size() != header.stringsCount)
        {
            LOG_ERROR("Invalid strings count in header: {}", header.stringsCount);
            return Common::RResult::Fail;
        }

        for(uint32_t i = 0; i < header.shadersCount; i++)
        {
            Asset::ShaderDesc assetShaderDesc;
            if(file.Read(reinterpret_cast<void*>(&assetShaderDesc), sizeof(assetShaderDesc)) != sizeof(assetShaderDesc))
            {
                LOG_ERROR("Failed to read shader header: {}", i);
                return Common::RResult::Fail;
            }

            auto shaderData = eastl::make_unique<std::byte[]>(assetShaderDesc.size);
            if(file.Read(reinterpret_cast<void*>(shaderData.get()), assetShaderDesc.size) != assetShaderDesc.size)
            {
                LOG_ERROR("Failed to read shader data: {}", i);
                return Common::RResult::Fail;
            }

            ShaderDesc shaderDesc;
            shaderDesc.name = getString(assetShaderDesc.nameIndex);
            shaderDesc.stage = assetShaderDesc.stage;
            shaderDesc.data = shaderData.get();
            shaderDesc.size = assetShaderDesc.size;

            shadersData.emplace_back(eastl::move(shaderData));
            shaders.emplace_back(eastl::move(shaderDesc));
        }

        auto shaderData = eastl::make_unique<std::byte[]>(header.srvSectionSize);
        if(file.Read(reinterpret_cast<void*>(shaderData.get()), header.srvSectionSize) != header.srvSectionSize)
        {
            LOG_ERROR("Failed to read SRV section size: {}", header.srvSectionSize);
            return Common::RResult::Fail;
        }

        auto uavData = eastl::make_unique<std::byte[]>(header.uavSectionSize);
        if(file.Read(reinterpret_cast<void*>(uavData.get()), header.uavSectionSize) != header.uavSectionSize)
        {
            LOG_ERROR("Failed to read UAV section size: {}", header.uavSectionSize);
            return Common::RResult::Fail;
        }

        auto cbvData = eastl::make_unique<std::byte[]>(header.cbvSectionSize);
        if(file.Read(reinterpret_cast<void*>(cbvData.get()), header.cbvSectionSize) != header.cbvSectionSize)
        {
            LOG_ERROR("Failed to read CBV section size: {}", header.cbvSectionSize);
            return Common::RResult::Fail;
        }

        effectsMap.reserve(header.effectsCount);
        for(uint32_t i = 0; i < header.effectsCount; i++)
        {
            Asset::EffectDesc assetEffectDesc;
            if(file.Read(reinterpret_cast<void*>(&assetEffectDesc), sizeof(assetEffectDesc)) != sizeof(assetEffectDesc))
            {
                LOG_ERROR("Failed to read effect header: {}", i);
                return Common::RResult::Fail;
            }
            auto name = getString(assetEffectDesc.nameIndex);
            auto hash = Common::Hash(name);
            ASSERT(effectsMap.contains(hash) == false);
            effectsMap.insert(std::make_pair(hash, i));

            EffectDesc effectDesc;
            effectDesc.name = name;

            for(uint32_t j = 0; j < assetEffectDesc.passCount; j++)
            {
                Asset::PassDesc assetPassDesc;
                if(file.Read(reinterpret_cast<void*>(&assetPassDesc), sizeof(assetPassDesc)) != sizeof(assetPassDesc))
                {
                    LOG_ERROR("Failed to read pass desc: {}", j);
                    return Common::RResult::Fail;
                }

                PassDesc passDesc;
                passDesc.name = getString(assetPassDesc.nameIndex);
                passDesc.rasterizerDesc = assetPassDesc.rasterizerDesc;
                passDesc.depthStencilDesc = assetPassDesc.depthStencilDesc;
                passDesc.blendDesc = assetPassDesc.blendDesc;

                eastl::fixed_vector<GAPI::ShaderStage, eastl::to_underlying(GAPI::ShaderStage::Count), false> shaderStages;

                for (uint32_t i = 0; i < eastl::to_underlying(GAPI::ShaderStage::Count); i++)
                {
                    const auto shaderStage = static_cast<GAPI::ShaderStage>(i);

                    if (IsSet(assetPassDesc.shaderStages, GetShaderStageMask(shaderStage)))
                        shaderStages.push_back(shaderStage);

                    passDesc.shaderIndexes[eastl::to_underlying(shaderStage)] = Asset::INVALID_INDEX;
                }

                eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count)> shaderIndexes;
                uint32_t shaderIndexesSize = sizeof(uint32_t) * shaderStages.size();
                if(file.Read(reinterpret_cast<void*>(&shaderIndexes), shaderIndexesSize) != shaderIndexesSize)
                {
                    LOG_ERROR("Failed to read shader indexes: {}", j);
                    return Common::RResult::Fail;
                }

                for(uint32_t i = 0; i < shaderStages.size(); i++)
                    passDesc.shaderIndexes[eastl::to_underlying(shaderStages[i])] = shaderIndexes[i];
/*
                Asset::ReflectionDesc::Header reflectionHeader;
                if(file.Read(reinterpret_cast<void*>(&reflectionHeader), sizeof(reflectionHeader)) != sizeof(reflectionHeader))
                {
                    LOG_ERROR("Failed to read reflection header: {}", j);
                    return Common::RResult::Fail;
                }

                passDesc.reflection.textureMetas.resize(reflectionHeader.textureMetasCount);
                if(file.Read(passDesc.reflection.textureMetas.data(), reflectionHeader.textureMetasCount * sizeof(GAPI::BindingLayoutTextureMeta)) != reflectionHeader.textureMetasCount * sizeof(GAPI::BindingLayoutTextureMeta))
                {
                    LOG_ERROR("Failed to read texture metas: {}", j);
                    return Common::RResult::Fail;
                }

                std::vector<Asset::FieldReflection> assetFields(reflectionHeader.variablesCount);
                if(file.Read(assetFields.data(), reflectionHeader.variablesCount * sizeof(Asset::FieldReflection)) != reflectionHeader.variablesCount * sizeof(Asset::FieldReflection))
                {
                    LOG_ERROR("Failed to read reflection variables: {}", j);
                    return Common::RResult::Fail;
                }

                passDesc.reflection.fields.reserve(reflectionHeader.variablesCount);
                for(const auto& field : assetFields)
                {
                    FieldReflection f;
                    f.name = getString(field.nameIndex);
                    f.type = field.type;
                    f.kind = field.kind;
                    f.arraySize = field.arraySize;
                    f.offset = field.offset;
                    f.size = field.size;
                    f.firstMemberIndex = field.firstMemberIndex;
                    f.memberCount = field.memberCount;
                    passDesc.reflection.fields.emplace_back(eastl::move(f));
                }

                std::vector<Asset::ResourceReflection> assetResourceReflections(reflectionHeader.resourcesCount);
                if(file.Read(assetResourceReflections.data(), reflectionHeader.resourcesCount * sizeof(Asset::ResourceReflection)) != reflectionHeader.resourcesCount * sizeof(Asset::ResourceReflection))
                {
                    LOG_ERROR("Failed to read reflection resources: {}", j);
                    return Common::RResult::Fail;
                }

                passDesc.reflection.resources.reserve(reflectionHeader.resourcesCount);
                for(const auto& resourceReflection : assetResourceReflections)
                {
                    ResourceReflection r;
                    r.name = getString(resourceReflection.nameIndex);
                    r.type = resourceReflection.type;
                    r.stageMask = resourceReflection.stageMask;
                    r.binding = resourceReflection.binding;
                    r.set = resourceReflection.set;
                    r.count = resourceReflection.count;
                    r.textureMetaIndex = resourceReflection.textureMetaIndex;
                    r.variables = resourceReflection.firstVarIndex != Asset::INVALID_INDEX ? eastl::span<FieldReflection>(passDesc.reflection.fields.data() + resourceReflection.firstVarIndex, resourceReflection.varCount) : eastl::span<FieldReflection>();
                    r.child = resourceReflection.firstChildResourceIndex != Asset::INVALID_INDEX ? &passDesc.reflection.resources[resourceReflection.firstChildResourceIndex] : nullptr;
                    r.next = resourceReflection.nextResourceIndex != Asset::INVALID_INDEX ? &passDesc.reflection.resources[resourceReflection.nextResourceIndex] : nullptr;
                    passDesc.reflection.resources.emplace_back(eastl::move(r));
                }


                passDesc.reflection.rootBlock = reflectionHeader.rootResourceIndex != Asset::INVALID_INDEX ? &passDesc.reflection.resources[reflectionHeader.rootResourceIndex] : nullptr;
               */
                effectDesc.passes.emplace_back(eastl::move(passDesc));
            }

            effects.emplace_back(eastl::move(effectDesc));
        }

        loaded = true;
        return Common::RResult::Ok;
    }

    bool EffectLibrary::GetEffectDesc(HashType hash, EffectDesc& effectDesc) const
    {
        ASSERT(loaded);

        auto it = effectsMap.find(hash);
        if(it == effectsMap.end())
            return false;

        effectDesc = effects[it->second];
        return true;
    }
}
