#include "ShaderCompiler.hpp"

#include "slang.h"
#include "slang-com-ptr.h"

#include "common/Result.hpp"
#include "gapi/Shader.hpp"
#include "SpirvToWgslTranscoder.hpp"

#include "reflection/ReflectionBuilder.hpp"
#include "SlangUtils.hpp"

#define TRY_SLANG(statement) \
	{ \
		SlangResult res = (statement); \
		if (SLANG_FAILED(res)) { \
            /* TODO cast error to RResult */ \
			return RR::Common::RResult::Fail; \
		} \
	}

#include <iostream>

namespace RR  {



    ShaderCompiler::ShaderCompiler() { }
    ShaderCompiler::~ShaderCompiler() { }

    Common::RResult ShaderCompiler::CompileShader(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const ShaderCompileDesc& desc, CompileResult& result)
    {
        ASSERT(desc.entryPoints.size() > 0);

        slang::TargetDesc targetDesc;
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
        targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
        targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;

        std::array<slang::CompilerOptionEntry, 2> compilerOptionEntries;
        compilerOptionEntries[0].name = slang::CompilerOptionName::NoMangle;
        compilerOptionEntries[0].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptionEntries[0].value.intValue0 = true;

        compilerOptionEntries[1].name = slang::CompilerOptionName::ForceDXLayout;
        compilerOptionEntries[1].value.kind = slang::CompilerOptionValueKind::Int;
        compilerOptionEntries[1].value.intValue0 = true;

        targetDesc.compilerOptionEntries = compilerOptionEntries.data();
        targetDesc.compilerOptionEntryCount = compilerOptionEntries.size();
     //   targetDesc.flags = SLANG_COMPILE_FLAG_NO_MANGLING;

        slang::SessionDesc sessionDesc;
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        std::vector<const char*> includePathes;
        for(auto& includePath : desc.includePathes)
            includePathes.emplace_back(includePath.c_str());

        sessionDesc.searchPaths = includePathes.data();
        sessionDesc.searchPathCount = includePathes.size();
        Slang::ComPtr<slang::ISession> session;
        TRY_SLANG(globalSession->createSession(sessionDesc, session.writeRef()));

        Slang::ComPtr<slang::IBlob> diagnostics;
        std::vector<slang::IComponentType*> componentTypes;
        std::vector<slang::IModule*> modules;

        modules.reserve(desc.modules.size());
        componentTypes.reserve(desc.modules.size() + desc.entryPoints.size());

        for (auto& moduleName : desc.modules)
        {
            slang::IModule* module = session->loadModule(moduleName.c_str(), diagnostics.writeRef());

            if (diagnostics)
            {
                std::string message = (const char*)diagnostics->getBufferPointer();
                std::cerr << "Could not load slang module from file '" + moduleName + "':\n" + message << std::endl;
                return Common::RResult::Fail;
            }

            if (!module)
                return Common::RResult::Fail;

            componentTypes.emplace_back(module);
            modules.emplace_back(std::move(module));
        }

        auto findEntryPoint = [&](const std::string& entryPointName, Slang::ComPtr<slang::IEntryPoint>& entryPointObj) {
            for (auto module : modules)
            {
                if (SLANG_SUCCEEDED(module->findEntryPointByName(entryPointName.c_str(), entryPointObj.writeRef())))
                    return Common::RResult::Ok;
            }

            std::cerr << "Could not find entry point: " << entryPointName << std::endl;
            return Common::RResult::Fail;
        };

        for (auto& entryPointName : desc.entryPoints)
        {
            Slang::ComPtr<slang::IEntryPoint> entryPointObj;
            RR_RETURN_ON_FAIL(findEntryPoint(entryPointName, entryPointObj));

            componentTypes.emplace_back(entryPointObj);
        }

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        if (SLANG_FAILED(session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                linkedProgram.writeRef(),
                diagnostics.writeRef())))
        {
            if (diagnostics)
            {
                std::string message = (const char*)diagnostics->getBufferPointer();
                std::cerr << "Could not link shaders with errors:\n" + message << std::endl;
                return Common::RResult::Fail;
            }

            std::cerr << "Could not link shaders with unknown error" << std::endl;
            return Common::RResult::Fail;
        }

        slang::ProgramLayout* programLayout = linkedProgram->getLayout(0, diagnostics.writeRef());

        if (diagnostics)
        {
            std::string message = (const char*)diagnostics->getBufferPointer();
            std::cerr << "Could not get layout with errors:\n" + message << std::endl;
            return Common::RResult::Fail;
        }

        if (!programLayout)
            return Common::RResult::Fail;

        for (uint32_t i = 0; i < programLayout->getEntryPointCount(); i++)
        {
            SlangStage stage = programLayout->getEntryPointByIndex(i)->getStage();

            ShaderResult shaderResult;
            shaderResult.stage = GetShaderStage(stage);
            shaderResult.session = session;
            shaderResult.name = programLayout->getEntryPointByIndex(i)->getName();
            if (SLANG_FAILED(linkedProgram->getEntryPointCode(i, 0, shaderResult.source.writeRef(), diagnostics.writeRef())))
            {
                if (diagnostics)
                {
                    std::string message = (const char*)diagnostics->getBufferPointer();
                    std::cerr << "Could not compile shaders with errors:\n" + message << std::endl;
                    return Common::RResult::Fail;
                }

                std::cerr << "Could not compile shaders with unknown error" << std::endl;
                return Common::RResult::Fail;
            }

            if (targetDesc.format == SLANG_SPIRV)
            {

                std::string wgslCode;
                RR_RETURN_ON_FAIL(SpirvToWgslTranscoder::Transcode(compiledCode, wgslCode));
                shaderResult.source.resize(wgslCode.size());
                std::memcpy(shaderResult.source.data(), wgslCode.data(), wgslCode.size());
            }
            else
            {
                // Copy to code to result
                shaderResult.source.resize(compiledCode->getBufferSize());
                std::memcpy(shaderResult.source.data(), compiledCode->getBufferPointer(), compiledCode->getBufferSize());
            }


            const char* c = reinterpret_cast<const char*>(shaderResult.source.data());
            size_t s = shaderResult.source.size();

            std::cout << "Shader: "<< std::endl << std::string(c,s ) << std::endl;

            // Анализируем скомпилированный HLSL код для извлечения правильной информации о binding
            std::string shaderCode(c, s);
            std::cout << "HLSL Analysis:" << std::endl;

            // Ищем cbuffer declarations
            size_t pos = 0;
            while ((pos = shaderCode.find("cbuffer", pos)) != std::string::npos)
            {
                size_t lineEnd = shaderCode.find('\n', pos);
                if (lineEnd != std::string::npos)
                {
                    std::string cbufferLine = shaderCode.substr(pos, lineEnd - pos);
                    std::cout << "  Found: " << cbufferLine << std::endl;

                    // Извлекаем имя буфера
                    size_t nameStart = cbufferLine.find(' ');
                    if (nameStart != std::string::npos)
                    {
                        nameStart++;
                        size_t nameEnd = cbufferLine.find(' ', nameStart);
                        if (nameEnd == std::string::npos) nameEnd = cbufferLine.find(':', nameStart);
                        if (nameEnd != std::string::npos)
                        {
                            std::string bufferName = cbufferLine.substr(nameStart, nameEnd - nameStart);
                            std::cout << "    Buffer name: " << bufferName << std::endl;
                        }
                    }

                    // Извлекаем register информацию
                    size_t registerPos = cbufferLine.find("register(");
                    if (registerPos != std::string::npos)
                    {
                        size_t registerStart = registerPos + 9; // "register("
                        size_t registerEnd = cbufferLine.find(')', registerStart);
                        if (registerEnd != std::string::npos)
                        {
                            std::string registerInfo = cbufferLine.substr(registerStart, registerEnd - registerStart);
                            std::cout << "    Register info: " << registerInfo << std::endl;
                        }
                    }
                }
                pos = lineEnd;
            }

            result.shaders.emplace_back(std::move(shaderResult));
        }


        std::cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ" << std::endl;

        ReflectionBuilder reflectionBuilder;
        reflectionBuilder.Build(programLayout);

        std::cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ" << std::endl;


        return Common::RResult::Ok;
    }
}