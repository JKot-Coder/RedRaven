#include "ShaderBuilder.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "JsonnetProcessor.hpp"
#include "ShaderCompiler.hpp"
#include "common/hashing/Hash.hpp"

#include "slang-com-ptr.h"
#include "slang.h"

#include <filesystem>
#include <fstream>

namespace RR
{
    using namespace Common;
    using namespace EffectLibrary;

    std::string toLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    ShaderBuilder::ShaderBuilder() : stringAllocator(1024) { }
    ShaderBuilder::~ShaderBuilder() { }

    GAPI::CullMode getCullMode(const std::string& cullMode)
    {
        switch (Hash(toLower(cullMode)))
        {
            case "none"_h: return GAPI::CullMode::None;
            case "front"_h: return GAPI::CullMode::Front;
            case "back"_h: return GAPI::CullMode::Back;
            default:
                throw std::runtime_error("Invalid cull mode: " + cullMode);
        }
    }

    GAPI::FillMode getFillMode(const std::string& fillMode)
    {
        switch (Hash(toLower(fillMode)))
        {
            case "solid"_h: return GAPI::FillMode::Solid;
            case "wireframe"_h: return GAPI::FillMode::Wireframe;
            default:
                throw std::runtime_error("Invalid fill mode: " + fillMode);
        }
    }

    GAPI::DepthStencilDesc::DepthAccess getDepthAccess(const std::string& depthAccess)
    {
        switch (Hash(toLower(depthAccess)))
        {
            case "none"_h: return GAPI::DepthStencilDesc::DepthAccess::None;
            case "read"_h: return GAPI::DepthStencilDesc::DepthAccess::Read;
            case "write"_h: return GAPI::DepthStencilDesc::DepthAccess::Write;
            case "readwrite"_h: return GAPI::DepthStencilDesc::DepthAccess::ReadWrite;
            default:
                throw std::runtime_error("Invalid depth access: " + depthAccess);
        }
    }

    GAPI::DepthStencilDesc::ComparisonFunc getComparisonFunc(const std::string& depthFunc)
    {
        switch (Hash(toLower(depthFunc)))
        {
            case "never"_h: return GAPI::DepthStencilDesc::ComparisonFunc::Never;
            case "always"_h: return GAPI::DepthStencilDesc::ComparisonFunc::Always;
            case "equal"_h: return GAPI::DepthStencilDesc::ComparisonFunc::Equal;
            case "notequal"_h: return GAPI::DepthStencilDesc::ComparisonFunc::NotEqual;
            case "less"_h: return GAPI::DepthStencilDesc::ComparisonFunc::Less;
            case "lessequal"_h: return GAPI::DepthStencilDesc::ComparisonFunc::LessEqual;
            case "greater"_h: return GAPI::DepthStencilDesc::ComparisonFunc::Greater;
            case "greaterequal"_h: return GAPI::DepthStencilDesc::ComparisonFunc::GreaterEqual;
            default:
                throw std::runtime_error("Invalid depth func: " + depthFunc);
        }
    }

    template <typename T, typename Cb>
    void evalIfExist(const nlohmann::json& json, Cb&& cb)
    {
        if (!json.empty())
            cb(json.get<T>());
    }

    void ShaderBuilder::evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc)
    {
        std::cout << "Evaluate render state desc: " << effect.dump() << std::endl;

        evalIfExist<std::string>(effect["cullMode"], [&](auto val) { rasterizerDesc.cullMode = getCullMode(val); });
        evalIfExist<std::string>(effect["fillMode"], [&](auto val) { rasterizerDesc.fillMode = getFillMode(val); });
        evalIfExist<std::string>(effect["depthAccess"], [&](auto val) { depthStencilDesc.depthAccess = getDepthAccess(val); });
        evalIfExist<std::string>(effect["depthFunc"], [&](auto val) { depthStencilDesc.depthFunc = getComparisonFunc(val); });
        depthStencilDesc.stencilReadMask = effect.value("stencilReadMask", false);

        auto colorWriteMasks = effect["colorWriteMasks"];
        if (colorWriteMasks.size() > GAPI::MAX_COLOR_ATTACHMENT_COUNT)
            throw std::runtime_error("Invalid color write masks count: " + std::to_string(colorWriteMasks.size()));

        for (size_t i = 0; i < colorWriteMasks.size(); i++)
        {
            uint32_t maskValue = colorWriteMasks[i].get<uint32_t>();
            ASSERT((maskValue & ~static_cast<uint32_t>(GAPI::WriteMask::All)) == 0); // TODO checks in release too.

            blendDesc.rtBlend[i].writeMask = static_cast<GAPI::WriteMask>(maskValue);
        }
    }

    uint32_t ShaderBuilder::pushString(std::string_view str)
    {
        auto it = stringsCache.find(str);
        if (it != stringsCache.end())
            return it->second;

        stringAllocator.allocateString(str);
        stringsCache[str] = stringsCount;
        return stringsCount++;
    };

    uint32_t ShaderBuilder::pushShader(ShaderResult&& shader)
    {
        uint32_t index = static_cast<uint32_t>(shaders.size());

        Asset::ShaderDesc shaderDesc;
        shaderDesc.header.nameIndex = pushString(shader.name);
        shaderDesc.header.stage = shader.stage;
        shaderDesc.header.size = static_cast<uint32_t>(shader.source->getBufferSize());
        shaderDesc.data = reinterpret_cast<const std::byte*>(shader.source->getBufferPointer());

        shaderResults.emplace_back(std::move(shader));
        shaders.emplace_back(std::move(shaderDesc));
        return index;
    }

    void ShaderBuilder::compileEffect(const std::string& name, nlohmann::json effect, const std::string& sourceFile)
    {
        std::cout << "Effect: " << name << std::endl;

        effects.push_back();
        EffectLibrary::Asset::EffectDesc& effectDesc = effects.back();
        effectDesc.header.nameIndex = pushString(name);
        auto& passes = effects.back().passes;

        for (auto& passKV : effect.items())
        {
            auto& passKey = passKV.key();
            auto& pass = passKV.value();
            std::cout << "  Pass: " << passKey << std::endl;

            nlohmann::json renderState = pass["renderState"];
            if (renderState.empty())
                throw std::runtime_error("Render state is empty for pass: " + passKey);

            EffectLibrary::Asset::PassDesc passDesc;
            passDesc.nameIndex = pushString(passKey);
            passDesc.shaderIndexes.fill(Asset::INVALID_INDEX);
            evaluateRenderStateDesc(renderState, passDesc.rasterizerDesc, passDesc.depthStencilDesc, passDesc.blendDesc);

            ShaderCompileDesc shaderCompileDesc;
            shaderCompileDesc.includePathes.emplace_back(std::filesystem::path(sourceFile).parent_path().generic_string());

            for (auto& module : pass["modules"].items())
                shaderCompileDesc.modules.emplace_back(module.value().get<std::string>());

            auto addEntryPoint = [&](const nlohmann::json& shader) {
                shaderCompileDesc.entryPoints.emplace_back(shader.get<std::string>());
            };

            if (!pass["vertexShader"].empty())
                addEntryPoint(pass["vertexShader"]);
            if (!pass["pixelShader"].empty())
                addEntryPoint(pass["pixelShader"]);

            // TODO check if entry points valid.

            ShaderCompiler compiler;
            CompileResult shaderResult;
            if (RR_FAILED(compiler.CompileShader(globalSession, shaderCompileDesc, shaderResult)))
                throw std::runtime_error("Failed to compile shader");

            for (auto& shader : shaderResult.shaders)
                passDesc.shaderIndexes[eastl::to_underlying(shader.stage)] = pushShader(std::move(shader));

            passes.push_back(std::move(passDesc));
        }

        effectDesc.header.passCount = static_cast<uint32_t>(passes.size());
    }

    Common::RResult ShaderBuilder::compileFile(const LibraryBuildDesc& desc, const std::string& sourceFile)
    {
        std::cout << "Compile file: " << sourceFile << std::endl;

        JsonnetProcessor jsonnetProcessor;
        nlohmann::json effectJson;
        auto result = jsonnetProcessor.evaluateFile(sourceFile, desc.includePathes, effectJson);
        if (result != Common::RResult::Ok)
        {
            std::cerr << "Failed to read effect file: " << sourceFile << std::endl;
            return Common::RResult::Fail;
        }

        try
        {
            nlohmann::json effectsJson = effectJson["effects"];

            for (auto& effect : effectsJson.items())
                compileEffect(effect.key(), effect.value(), sourceFile);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Effect processing failed with error: " << e.what() << std::endl;
            return Common::RResult::Fail;
        }

        return Common::RResult::Ok;
    }

    Common::RResult ShaderBuilder::BuildLibrary(const LibraryBuildDesc& desc)
    {
        std::cout << "Build shader library: " << desc.inputFile << " -> " << desc.outputFile << std::endl;

        JsonnetProcessor jsonnetProcessor;
        nlohmann::json outputJson;
        auto result = jsonnetProcessor.evaluateFile(desc.inputFile, desc.includePathes, outputJson);
        if (result != Common::RResult::Ok)
        {
            std::cerr << "Failed to evaluate build list file: " << desc.inputFile << std::endl;
            return Common::RResult::Fail;
        }

        auto sources = outputJson["Sources"];
        if (sources.empty())
        {
            std::cerr << "No sources found in build list file: " << desc.inputFile << std::endl;
            return Common::RResult::Fail;
        }

        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef())))
        {
            std::cerr << "Failed to create global session:";
            return Common::RResult::Fail;
        }

        for (auto& source : sources)
        {
            if (compileFile(desc, source.get<std::string>()) != Common::RResult::Ok)
            {
                std::cerr << "Failed to compile effect: " << source << std::endl;
                return Common::RResult::Fail;
            }
        }

        if (saveLibrary(desc) != Common::RResult::Ok)
        {
            std::cerr << "Failed to save shader library: " << desc.outputFile << std::endl;
            return Common::RResult::Fail;
        }

        return Common::RResult::Ok;
    }

    Common::RResult ShaderBuilder::saveLibrary(const LibraryBuildDesc& desc)
    {
        std::cout << "Save shader library: " << desc.outputFile << std::endl;

        std::ofstream file(desc.outputFile, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << desc.outputFile << std::endl;
            return Common::RResult::Fail;
        }

        uint32_t stringSectionSize = 0;
        for (auto& chunk : stringAllocator)
            stringSectionSize += static_cast<uint32_t>(chunk.allocated);

        uint32_t shadersSectionSize = 0;
        for (auto& shader : shaders)
            shadersSectionSize += shader.header.size + sizeof(shader.header);

        uint32_t effectsSectionSize = 0;
        for (auto& effect : effects)
            effectsSectionSize += sizeof(Asset::EffectDesc::Header) + static_cast<uint32_t>(effect.passes.size()) * sizeof(EffectLibrary::Asset::PassDesc);

        Asset::Header header;
        header.magic = Asset::Header::MAGIC;
        header.version = Asset::Header::VERSION;
        header.stringSectionSize = stringSectionSize;
        header.stringsCount = stringsCount;
        header.shadersSectionSize = shadersSectionSize;
        header.shadersCount = static_cast<uint32_t>(shaders.size());
        header.effectsSectionSize = effectsSectionSize;
        header.effectsCount = static_cast<uint32_t>(effects.size());

        // Header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Strings
        for (auto& chunk : stringAllocator)
            file.write(reinterpret_cast<const char*>(chunk.buffer.get()), chunk.allocated);

        // Shaders
        for (auto& shader : shaders)
        {
            file.write(reinterpret_cast<const char*>(&shader.header), sizeof(Asset::ShaderDesc::Header));
            file.write(reinterpret_cast<const char*>(shader.data), shader.header.size);
        }

        // Effects
        for (auto& effect : effects)
        {
            file.write(reinterpret_cast<const char*>(&effect.header), sizeof(Asset::EffectDesc::Header));

            for (auto& pass : effect.passes)
                file.write(reinterpret_cast<const char*>(&pass), sizeof(Asset::PassDesc));
        }

        shaderResults.clear();

        file.close();
        return Common::RResult::Ok;
    }

}