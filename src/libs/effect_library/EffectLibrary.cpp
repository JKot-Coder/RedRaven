#include "EffectLibrary.hpp"
#include "EffectFormat.hpp"

#include "common/io/File.hpp"
#include "common/Result.hpp"
#include "common/OnScopeExit.hpp"
#include "common/hashing/Default.hpp"

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
            Asset::ShaderDesc::Header header;
            if(file.Read(reinterpret_cast<void*>(&header), sizeof(header)) != sizeof(header))
            {
                LOG_ERROR("Failed to read shader header: {}", i);
                return Common::RResult::Fail;
            }

            auto shaderData = eastl::make_unique<std::byte[]>(header.size);
            if(file.Read(reinterpret_cast<void*>(shaderData.get()), header.size) != header.size)
            {
                LOG_ERROR("Failed to read shader data: {}", i);
                return Common::RResult::Fail;
            }

            ShaderDesc shaderDesc;
            shaderDesc.name = getString(header.nameIndex);
            shaderDesc.type = header.type;
            shaderDesc.data = shaderData.get();
            shaderDesc.size = header.size;

            shadersData.emplace_back(eastl::move(shaderData));
            shaders.emplace_back(eastl::move(shaderDesc));
        }

        effectsMap.reserve(header.effectsCount);
        for(uint32_t i = 0; i < header.effectsCount; i++)
        {
            Asset::EffectDesc::Header header;
            if(file.Read(reinterpret_cast<void*>(&header), sizeof(header)) != sizeof(header))
            {
                LOG_ERROR("Failed to read effect header: {}", i);
                return Common::RResult::Fail;
            }
            auto name = getString(header.nameIndex);
            auto hash = Common::Hashing::Default::Hash(name);
            ASSERT(effectsMap.contains(hash) == false);
            effectsMap.insert(std::make_pair(hash, i));

            EffectDesc effectDesc;
            effectDesc.name = name;

            for(uint32_t j = 0; j < header.passCount; j++)
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
                passDesc.shaderIndexes = assetPassDesc.shaderIndexes;

                effectDesc.passes.emplace_back(eastl::move(passDesc));
            }

            effects.emplace_back(eastl::move(effectDesc));
        }

        loaded = true;
        return Common::RResult::Ok;
    }

    bool EffectLibrary::GetEffectDesc(Common::Hashing::Default::HashType hash, EffectDesc& effectDesc) const
    {
        ASSERT(loaded);

        auto it = effectsMap.find(hash);
        if(it == effectsMap.end())
            return false;

        effectDesc = effects[it->second];
        return true;
    }
}