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

        stringsData = eastl::make_unique<std::byte[]>(header.stringSectionSize);
        if(file.Read(reinterpret_cast<void*>(stringsData.get()), header.stringSectionSize) != header.stringSectionSize)
        {
            LOG_ERROR("Failed to read strings data: {}", header.stringSectionSize);
            return Common::RResult::Fail;
        }

        char* lastChar = reinterpret_cast<char*>(stringsData.get() + header.stringSectionSize);
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
            Asset::ShaderDesc::Header shaderHeader;
            if(file.Read(reinterpret_cast<void*>(&shaderHeader), sizeof(shaderHeader)) != sizeof(shaderHeader))
            {
                LOG_ERROR("Failed to read shader header: {}", i);
                return Common::RResult::Fail;
            }

            auto shaderData = eastl::make_unique<std::byte[]>(shaderHeader.size);
            if(file.Read(reinterpret_cast<void*>(shaderData.get()), shaderHeader.size) != shaderHeader.size)
            {
                LOG_ERROR("Failed to read shader data: {}", i);
                return Common::RResult::Fail;
            }

            ShaderDesc shaderDesc;
            shaderDesc.name = getString(shaderHeader.nameIndex);
            shaderDesc.stage = shaderHeader.stage;
            shaderDesc.data = shaderData.get();
            shaderDesc.size = shaderHeader.size;

            shadersData.emplace_back(eastl::move(shaderData));
            shaders.emplace_back(eastl::move(shaderDesc));
        }

        effectsMap.reserve(header.effectsCount);
        for(uint32_t i = 0; i < header.effectsCount; i++)
        {
            Asset::EffectDesc::Header effectHeader;
            if(file.Read(reinterpret_cast<void*>(&effectHeader), sizeof(effectHeader)) != sizeof(effectHeader))
            {
                LOG_ERROR("Failed to read effect header: {}", i);
                return Common::RResult::Fail;
            }
            auto name = getString(effectHeader.nameIndex);
            auto hash = Common::Hash(name);
            ASSERT(effectsMap.contains(hash) == false);
            effectsMap.insert(std::make_pair(hash, i));

            EffectDesc effectDesc;
            effectDesc.name = name;

            for(uint32_t j = 0; j < effectHeader.passCount; j++)
            {
                Asset::PassDesc assetPassDesc;
                if(file.Read(reinterpret_cast<void*>(&assetPassDesc), sizeof(assetPassDesc)) != sizeof(assetPassDesc))
                {
                    LOG_ERROR("Failed to read pass desc: {}", j);
                    return Common::RResult::Fail;
                }

                PassDesc passDesc;
                passDesc.name = getString(assetPassDesc.nameIndex);
                passDesc.rasterizerDesc = assetPassDesc.psoDesc.rasterizerDesc;
                passDesc.depthStencilDesc = assetPassDesc.psoDesc.depthStencilDesc;
                passDesc.blendDesc = assetPassDesc.psoDesc.blendDesc;
                passDesc.shaderIndexes = assetPassDesc.psoDesc.shaderIndexes;

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
