#include "EffectLibrary.hpp"
#include "EffectFormat.hpp"

#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"
#include "common/hashing/Hash.hpp"
#include "common/io/File.hpp"

#include "render/DeviceContext.hpp"

#define CHECK_RETURN_FAIL(expr)                    \
    do                                             \
    {                                              \
        ASSERT(expr);                              \
        if (!(expr)) return Common::RResult::Fail; \
    } while (0)

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
        if (file.Read(reinterpret_cast<void*>(&header), sizeof(header)) != sizeof(header))
        {
            LOG_ERROR("Failed to read header from file: {}", path);
            return Common::RResult::Fail;
        }

        if (header.magic != Asset::Header::MAGIC)
        {
            LOG_ERROR("Invalid magic number in header: {}", header.magic);
            return Common::RResult::Fail;
        }

        if (header.version != Asset::Header::VERSION)
        {
            LOG_ERROR("Invalid version in header: {}", header.version);
            return Common::RResult::Fail;
        }

        stringsData = eastl::make_unique<std::byte[]>(header.stringsSectionSize);
        if (file.Read(reinterpret_cast<void*>(stringsData.get()), header.stringsSectionSize) != header.stringsSectionSize)
        {
            LOG_ERROR("Failed to read strings data: {}", header.stringsSectionSize);
            return Common::RResult::Fail;
        }

        char* lastChar = reinterpret_cast<char*>(stringsData.get() + header.stringsSectionSize);
        char* currentChar = reinterpret_cast<char*>(stringsData.get());
        char* stringStart = currentChar;

        strings.reserve(header.stringsCount);
        while (currentChar != lastChar)
        {
            if (*currentChar == '\0')
            {
                strings.push_back(stringStart);
                stringStart = currentChar + 1;
            }

            currentChar++;
        }

        if (strings.size() != header.stringsCount)
        {
            LOG_ERROR("Invalid strings count in header: {}", header.stringsCount);
            return Common::RResult::Fail;
        }

        for (uint32_t i = 0; i < header.shadersCount; i++)
        {
            Asset::ShaderDesc assetShaderDesc;
            if (file.Read(reinterpret_cast<void*>(&assetShaderDesc), sizeof(assetShaderDesc)) != sizeof(assetShaderDesc))
            {
                LOG_ERROR("Failed to read shader header: {}", i);
                return Common::RResult::Fail;
            }

            auto shaderData = eastl::make_unique<std::byte[]>(assetShaderDesc.size);
            if (file.Read(reinterpret_cast<void*>(shaderData.get()), assetShaderDesc.size) != assetShaderDesc.size)
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

        auto srvRelections = eastl::vector<Asset::SrvReflection>(header.srvCount);
        CHECK_RETURN_FAIL(header.srvCount * sizeof(Asset::SrvReflection) == header.srvSectionSize);
        if (file.Read(reinterpret_cast<void*>(srvRelections.data()), header.srvSectionSize) != header.srvSectionSize)
        {
            LOG_ERROR("Failed to read SRV section size: {}", header.srvSectionSize);
            return Common::RResult::Fail;
        }

        auto uavReflections = eastl::vector<Asset::UavReflection>(header.uavCount);
        CHECK_RETURN_FAIL(header.uavCount * sizeof(Asset::UavReflection) == header.uavSectionSize);
        if (file.Read(reinterpret_cast<void*>(uavReflections.data()), header.uavSectionSize) != header.uavSectionSize)
        {
            LOG_ERROR("Failed to read UAV section size: {}", header.uavSectionSize);
            return Common::RResult::Fail;
        }

        auto cbvReflections = eastl::vector<Asset::CbvReflection>(header.cbvCount);
        CHECK_RETURN_FAIL(header.cbvCount * sizeof(Asset::CbvReflection) == header.cbvSectionSize);
        if (file.Read(reinterpret_cast<void*>(cbvReflections.data()), header.cbvSectionSize) != header.cbvSectionSize)
        {
            LOG_ERROR("Failed to read CBV section size: {}", header.cbvSectionSize);
            return Common::RResult::Fail;
        }

        eastl::vector<ResourceReflection> tempResources;
        tempResources.reserve(header.srvCount + header.uavCount + header.cbvCount);

        absl::flat_hash_map<uint32_t, const ResourceReflection*> resourcesMap;
        resourcesMap.reserve(header.srvCount + header.uavCount + header.cbvCount);

        for (uint32_t i = 0; i < header.srvCount; i++)
        {
            const auto& src = srvRelections[i];
            auto& dst = tempResources.emplace_back();
            dst.type = Asset::ResourceType::SRV;
            dst.name = getString(src.nameIndex);
            dst.usageMask = src.usageMask;
            dst.binding = src.binding;
            dst.count = src.count;
            dst.dimension = src.dimension;
            dst.sampleType = src.sampleType;
            dst.format = {};
            dst.layoutIndex = Asset::INVALID_INDEX;
            resourcesMap.emplace(Asset::MakeResourceId(Asset::ResourceType::SRV, i), &dst);
        }

        for (uint32_t i = 0; i < header.uavCount; i++)
        {
            const auto& src = uavReflections[i];
            auto& dst = tempResources.emplace_back();
            dst.type = Asset::ResourceType::UAV;
            dst.name = getString(src.nameIndex);
            dst.usageMask = src.usageMask;
            dst.binding = src.binding;
            dst.count = src.count;
            dst.dimension = src.dimension;
            dst.sampleType = {};
            dst.format = src.format;
            dst.layoutIndex = Asset::INVALID_INDEX;
            resourcesMap.emplace(Asset::MakeResourceId(Asset::ResourceType::UAV, i), &dst);
        }

        for (uint32_t i = 0; i < header.cbvCount; i++)
        {
            const auto& src = cbvReflections[i];
            auto& dst = tempResources.emplace_back();
            dst.type = Asset::ResourceType::CBV;
            dst.name = getString(src.nameIndex);
            dst.usageMask = src.usageMask;
            dst.binding = src.binding;
            dst.count = src.count;
            dst.dimension = {};
            dst.sampleType = {};
            dst.format = {};
            dst.layoutIndex = src.layoutIndex;
            resourcesMap.emplace(Asset::MakeResourceId(Asset::ResourceType::CBV, i), &dst);
        }

        CHECK_RETURN_FAIL(header.layoutsSectionSize % sizeof(uint32_t) == 0);
        eastl::vector<uint32_t> layoutsData(header.layoutsSectionSize / sizeof(uint32_t));
        if (file.Read(reinterpret_cast<void*>(layoutsData.data()), header.layoutsSectionSize) != header.layoutsSectionSize)
        {
            LOG_ERROR("Failed to read layouts section size: {}", header.layoutsSectionSize);
            return Common::RResult::Fail;
        }

        using LayoutSpan = eastl::span<uint32_t>;
        auto getLayout = [&layoutsData, &header](uint32_t index, LayoutSpan& span) -> Common::RResult {
            span = LayoutSpan();
            CHECK_RETURN_FAIL(index != Asset::INVALID_INDEX);
            CHECK_RETURN_FAIL(index < layoutsData.size());
            CHECK_RETURN_FAIL(index < header.layoutsCount);
            const uint32_t size = layoutsData[index];

            if (size == 0)
                return RResult::Ok;

            CHECK_RETURN_FAIL(index + size < layoutsData.size());
            uint32_t* begin = layoutsData.data() + index + 1;
            span = eastl::span<uint32_t>(begin, size);

            return Common::RResult::Ok;
        };

        auto bindGroupsData = eastl::make_unique<std::byte[]>(header.bindGroupsSectionSize);
        if (file.Read(reinterpret_cast<void*>(bindGroupsData.get()), header.bindGroupsSectionSize) != header.bindGroupsSectionSize)
        {
            LOG_ERROR("Failed to read bind groups section size: {}", header.bindGroupsSectionSize);
            return Common::RResult::Fail;
        }

        CHECK_RETURN_FAIL(header.bindGroupsCount * sizeof(Asset::BindGroup) == header.bindGroupsSectionSize);
        auto bindGroupsAssets = eastl::span<Asset::BindGroup>(reinterpret_cast<Asset::BindGroup*>(bindGroupsData.get()), header.bindGroupsCount);

        resources.reserve(tempResources.size());
        bindingGroupReflections.reserve(header.bindGroupsCount);

        for (const auto& bindGroupAsset : bindGroupsAssets)
        {
            BindingGroupReflection bindingGroupReflection;
            bindingGroupReflection.name = getString(bindGroupAsset.nameIndex);
            bindingGroupReflection.bindingSpace = bindGroupAsset.bindingSpace;

            LayoutSpan layout;
            RR_RETURN_ON_FAIL(getLayout(bindGroupAsset.resourcesLayoutIndex, layout));

            const auto spanStart = resources.size();
            for (const uint32_t id : layout)
            {
                auto it = resourcesMap.find(id);
                CHECK_RETURN_FAIL(it != resourcesMap.end());
                resources.emplace_back(*it->second);
            }
            bindingGroupReflection.resources = eastl::span<ResourceReflection>(
                resources.data() + spanStart, resources.size() - spanStart);

            bindingGroupReflections.emplace_back(eastl::move(bindingGroupReflection));
        }

        effectsMap.reserve(header.effectsCount);
        for (uint32_t i = 0; i < header.effectsCount; i++)
        {
            Asset::EffectDesc assetEffectDesc;
            if (file.Read(reinterpret_cast<void*>(&assetEffectDesc), sizeof(assetEffectDesc)) != sizeof(assetEffectDesc))
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

            for (uint32_t j = 0; j < assetEffectDesc.passCount; j++)
            {
                Asset::PassDesc assetPassDesc;
                if (file.Read(reinterpret_cast<void*>(&assetPassDesc), sizeof(assetPassDesc)) != sizeof(assetPassDesc))
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
                if (file.Read(reinterpret_cast<void*>(&shaderIndexes), shaderIndexesSize) != shaderIndexesSize)
                {
                    LOG_ERROR("Failed to read shader indexes: {}", j);
                    return Common::RResult::Fail;
                }

                for (uint32_t i = 0; i < shaderStages.size(); i++)
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
        if (it == effectsMap.end())
            return false;

        effectDesc = effects[it->second];
        return true;
    }
}
