#include "ShaderBuilder.hpp"

#include <iostream>
#include <algorithm>
#include <cctype>

#include "JsonnetProcessor.hpp"
#include "ShaderCompiler.hpp"
#include "common/hashing/Default.hpp"

#include <filesystem>

namespace RR
{
    using namespace Common::Hashing::Default;

    std::string toLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    ShaderBuilder::ShaderBuilder() { }
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
        if(!json.empty())
            cb(json.get<T>());
    }

    void ShaderBuilder::evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc)
    {
        std::cout << "Evaluate render state desc: " << effect.dump() << std::endl;

        evalIfExist<std::string>(effect["cullMode"], [&](auto val){ rasterizerDesc.cullMode = getCullMode(val); } );
        evalIfExist<std::string>(effect["fillMode"], [&](auto val){ rasterizerDesc.fillMode = getFillMode(val); } );
        evalIfExist<std::string>(effect["depthAccess"], [&](auto val){ depthStencilDesc.depthAccess = getDepthAccess(val); } );
        evalIfExist<std::string>(effect["depthFunc"], [&](auto val){ depthStencilDesc.depthFunc = getComparisonFunc(val); } );
        evalIfExist<bool>(effect["stencilEnabled"], [&](auto val){ depthStencilDesc.stencilEnabled = val; } );

        auto colorWriteMasks = effect["colorWriteMasks"];
        if(colorWriteMasks.size() > GAPI::MAX_RENDER_TARGETS_COUNT)
            throw std::runtime_error("Invalid color write masks count: " + std::to_string(colorWriteMasks.size()));

        for(size_t i = 0; i < colorWriteMasks.size(); i++)
        {
            uint32_t maskValue = colorWriteMasks[i].get<uint32_t>();
            ASSERT((maskValue & ~static_cast<uint32_t>(GAPI::WriteMask::All)) == 0); // TODO checks in release too.

            blendDesc.rtBlend[i].writeMask = static_cast<GAPI::WriteMask>(maskValue);
        }
    }

    Common::RResult ShaderBuilder::compileEffect(const LibraryBuildDesc& desc, const std::string& sourceFile)
    {
        std::cout << "Compile compile effect: " << sourceFile << std::endl;

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
            for(auto& effect : effectJson.items())
            {
                std::cout << "Effect: " << effect.key() << std::endl;

                for(auto& passKV :effect.value().items())
                {
                    auto& passKey = passKV.key();
                    auto& pass = passKV.value();
                    std::cout << "  Pass: " << passKey << std::endl;

                    nlohmann::json renderState = pass["renderState"];
                    if(renderState.empty())
                        throw std::runtime_error("Render state is empty for pass: " + passKey);

                    GAPI::RasterizerDesc rasterizerDesc;
                    GAPI::DepthStencilDesc depthStencilDesc;
                    GAPI::BlendDesc blendDesc;
                    evaluateRenderStateDesc(renderState, rasterizerDesc, depthStencilDesc, blendDesc);

                    ShaderCompileDesc shaderCompileDesc;
                    shaderCompileDesc.includePathes.emplace_back(std::filesystem::path(sourceFile).parent_path().generic_string());

                    for(auto& module : pass["modules"].items())
                        shaderCompileDesc.modules.emplace_back(module.value().get<std::string>());

                    auto shader = pass["vertexShader"];

                    auto addEntryPoint = [&](const nlohmann::json& shader) {
                        shaderCompileDesc.entryPoints.emplace_back(shader.get<std::string>());
                    };

                    if (!pass["vertexShader"].empty())
                        addEntryPoint(pass["vertexShader"]);
                    if (!pass["pixelShader"].empty())
                        addEntryPoint(pass["pixelShader"]);

                    ShaderCompiler compiler;
                    if (RR_FAILED(compiler.CompileShader(shaderCompileDesc)))
                        throw std::runtime_error("Failed to compile shader");
                }
            }
        }
        catch(const std::exception& e)
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
        if(sources.empty())
        {
            std::cerr << "No sources found in build list file: " << desc.inputFile << std::endl;
            return Common::RResult::Fail;
        }

        for (auto& source : sources)
        {
            if(compileEffect(desc, source.get<std::string>()) != Common::RResult::Ok)
            {
                std::cerr << "Failed to compile effect: " << source << std::endl;
                return Common::RResult::Fail;
            }
        }

        return Common::RResult::Ok;
    }
}