#include "EffectLibrary.hpp"
#include "EffectFormat.hpp"

#include "common/io/File.hpp"
#include "common/Result.hpp"
#include "common/OnScopeExit.hpp"

#include "render/DeviceContext.hpp"

namespace RR::EffectAsset
{
    Common::RResult EffectLibrary::Load(const std::string& path)
    {
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

        Header header;
        if(file.Read(reinterpret_cast<void*>(&header), sizeof(header)) != sizeof(header))
        {
            LOG_ERROR("Failed to read header from file: {}", path);
            return Common::RResult::Fail;
        }

        if(header.magic != Header::MAGIC)
        {
            LOG_ERROR("Invalid magic number in header: {}", header.magic);
            return Common::RResult::Fail;
        }

        if(header.version != Header::VERSION)
        {
            LOG_ERROR("Invalid version in header: {}", header.version);
            return Common::RResult::Fail;
        }

        stringsData = std::make_unique<std::byte[]>(header.stringSectionSize);
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
            uint32_t shaderSize;
            if(file.Read(reinterpret_cast<void*>(&shaderSize), sizeof(shaderSize)) != sizeof(shaderSize))
            {
                LOG_ERROR("Failed to read shader size: {}", i);
                return Common::RResult::Fail;
            }

            auto shaderData = std::make_unique<std::byte[]>(shaderSize);
            if(file.Read(reinterpret_cast<void*>(shaderData.get()), shaderSize) != shaderSize)
            {
                LOG_ERROR("Failed to read shader data: {}", i);
                return Common::RResult::Fail;
            }

            Render::DeviceContext::Instance().CreateShader(GAPI::ShaderDesc{GAPI::ShaderType::Vertex, shaderData.get(), shaderSize}, "shader_" + std::to_string(i));
        }

        return Common::RResult::Ok;
    }
}