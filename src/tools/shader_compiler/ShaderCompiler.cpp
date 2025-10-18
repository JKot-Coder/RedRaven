#include "ShaderCompiler.hpp"

#include "slang.h"
#include "slang-com-ptr.h"

#include "common/Result.hpp"
#include "gapi/Shader.hpp"

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

    namespace {
        GAPI::ShaderType getShaderType(SlangStage stage)
        {
            switch (stage)
            {
                case SLANG_STAGE_VERTEX: return GAPI::ShaderType::Vertex;
                case SLANG_STAGE_HULL: return GAPI::ShaderType::Hull;
                case SLANG_STAGE_DOMAIN: return GAPI::ShaderType::Domain;
                case SLANG_STAGE_GEOMETRY: return GAPI::ShaderType::Geometry;
                case SLANG_STAGE_FRAGMENT: return GAPI::ShaderType::Pixel;
                case SLANG_STAGE_COMPUTE: return GAPI::ShaderType::Compute;
                case SLANG_STAGE_RAY_GENERATION: return GAPI::ShaderType::RayGen;
                case SLANG_STAGE_INTERSECTION: return GAPI::ShaderType::RayIntersection;
                case SLANG_STAGE_ANY_HIT: return GAPI::ShaderType::RayAnyHit;
                case SLANG_STAGE_CLOSEST_HIT: return GAPI::ShaderType::RayClosestHit;
                case SLANG_STAGE_MISS: return GAPI::ShaderType::RayMiss;
                case SLANG_STAGE_CALLABLE: return GAPI::ShaderType::Callable;
                case SLANG_STAGE_MESH: return GAPI::ShaderType::Mesh;
                case SLANG_STAGE_AMPLIFICATION: return GAPI::ShaderType::Amplification;
                default: throw std::runtime_error("Unknown shader stage");
            }
        }

        void printOffset(
            slang::VariableLayoutReflection* varLayout,
            slang::ParameterCategory layoutUnit)
        {
            size_t spaceOffset = varLayout->getBindingSpace(layoutUnit);

            switch(layoutUnit)
            {
            default:
                break;

            case slang::ParameterCategory::ConstantBuffer:
            case slang::ParameterCategory::ShaderResource:
            case slang::ParameterCategory::UnorderedAccess:
            case slang::ParameterCategory::SamplerState:
            case slang::ParameterCategory::DescriptorTableSlot:
                std::cout << "space: " << spaceOffset << std::endl;
            }
        }

        void printRelativeOffsets(
            slang::VariableLayoutReflection* varLayout)
        {
            std::cout << "relative offset: " << std::endl;
            int usedLayoutUnitCount = varLayout->getCategoryCount();
            for (int i = 0; i < usedLayoutUnitCount; ++i)
            {
                auto layoutUnit = varLayout->getCategoryByIndex(i);
                printOffset(varLayout, layoutUnit);
            }
        }


        void printTypeLayout(slang::TypeLayoutReflection* typeLayout, std::string indent = "")
        {
            auto kind = typeLayout->getKind();
            switch (kind)
            {
                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                case slang::TypeReflection::Kind::TextureBuffer:
                case slang::TypeReflection::Kind::ShaderStorageBuffer:
                    {
                        std::cout << indent << (kind == slang::TypeReflection::Kind::ConstantBuffer ? "ConstantBuffer" : kind == slang::TypeReflection::Kind::ParameterBlock ? "ParameterBlock" : kind == slang::TypeReflection::Kind::TextureBuffer ? "TextureBuffer" : "ShaderStorageBuffer") << " : " << std::endl;
                     //   printRelativeOffsets(elementVarLayout);
                        printRelativeOffsets(typeLayout->getContainerVarLayout());

                        auto elementVarLayout = typeLayout->getElementVarLayout();
                      //  printRelativeOffsets(elementVarLayout);
                        /*   std::cout << "element: " << std::endl;
                        //printOffsets(elementVarLayout);*/

                        //std::cout << indent << "type layout: " << std::endl;
                        printTypeLayout(
                            elementVarLayout->getTypeLayout(), indent + "  ");
                    }
                    break;
                case slang::TypeReflection::Kind::Struct:
                {
                    std::cout << indent << "Struct: " << std::endl;

                    uint32_t paramCount = typeLayout->getFieldCount();
                    for (uint32_t i = 0; i < paramCount; i++)
                    {
                        std::cout << indent << "-" << typeLayout->getFieldByIndex(i)->getName() << std::endl;
                        printTypeLayout( typeLayout->getFieldByIndex(i)->getTypeLayout(), indent + "  ");
                    }
                }
                break;
                case slang::TypeReflection::Kind::Scalar:
                case slang::TypeReflection::Kind::Matrix:
                {
                    std::cout << indent <<  typeLayout->getName() << std::endl;
                }
                break;
                default:
                    std::cerr << indent << "Unknown global params type" << std::endl;
                    break;
            }
        }
    }

    ShaderCompiler::ShaderCompiler() { }
    ShaderCompiler::~ShaderCompiler() { }


    std::string getBindingTypeStr(slang::BindingType bindingType)
    {
        switch (bindingType)
        {
            case slang::BindingType::ConstantBuffer: return "ConstantBuffer";
            case slang::BindingType::ParameterBlock: return "ParameterBlock";
            case slang::BindingType::PushConstant: return "PushConstant";
            default: return "Unknown";
        }
    }

    std::string getParameterCategoryStr(slang::ParameterCategory parameterCategory)
    {
        switch (parameterCategory)
        {
            case slang::ParameterCategory::SubElementRegisterSpace: return "SubElementRegisterSpace";
            case slang::ParameterCategory::ConstantBuffer: return "ConstantBuffer";
            case slang::ParameterCategory::Uniform: return "Uniform";
            default: return "Unknown";
        }
    }

    void CollectSets(slang::ShaderReflection* shaderReflection)
    {
     //   slang::VariableLayoutReflection* scopeVarLayout = shaderReflection->getGlobalParamsVarLayout();
	//	slang::TypeLayoutReflection* typeLayoutReflection = scopeVarLayout->getTypeLayout();

		const uint32_t paramCount = shaderReflection->getParameterCount();
		for (uint32_t i = 0; i < paramCount; i++)
        {
			auto param = shaderReflection->getParameterByIndex(i);
         //   auto bindingRangeType = typeLayoutReflection->getBindingRangeType(i);
			auto paramName = param->getName();
			//auto type = param->getType();

            // Получаем binding info
            slang::ParameterCategory category = param->getCategory();
            uint32_t binding = param->getBindingIndex();
            uint32_t space = param->getBindingSpace();

            std::cout << "Param " << i << ": " << paramName << std::endl;
            std::cout << "  Category: " << getParameterCategoryStr(category) << std::endl;
            std::cout << "  Binding: " << binding << std::endl;
            std::cout << "  Space: " << space << std::endl;

            //std::cout << "Parameter: " << paramName << " " << getBindingTypeStr(bindingRangeType) << std::endl;
        }


    }


    Common::RResult ShaderCompiler::CompileShader(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const ShaderCompileDesc& desc, CompileResult& result)
    {
        slang::TargetDesc targetDesc;
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
        targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
        targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
        targetDesc.flags = 0;

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
            shaderResult.type = getShaderType(stage);
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

            result.shaders.emplace_back(std::move(shaderResult));
        }

        slang::VariableLayoutReflection* globalParamsVarLayout = programLayout->getGlobalParamsVarLayout();

        CollectSets(programLayout);

        auto scopeTypeLayout = globalParamsVarLayout->getTypeLayout();
        printTypeLayout(scopeTypeLayout);


        return Common::RResult::Ok;
    }
}