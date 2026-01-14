#include "ReflectionBuilder.hpp"

#include "slang.h"
using namespace slang;

#include <iostream>

#include "ProgramReflection.hpp"
#include "ProgramVersion.hpp"
using namespace Falcor;

#include "common/Result.hpp"
#include "gapi/Shader.hpp"

#include "SlangUtils.hpp"

#include "effect_library/EffectFormat.hpp"
#include "EffectSerializer.hpp"

#define THROW(message)          \
    ASSERT_MSG(false, message); \
    throw std::runtime_error(message);

namespace RR
{
    namespace
    {
        EffectLibrary::Asset::FieldType getFieldType(ReflectionBasicType::Type type)
        {
            switch (type)
            {
                case ReflectionBasicType::Type::Bool: return EffectLibrary::Asset::FieldType::Bool;
                case ReflectionBasicType::Type::Bool2: return EffectLibrary::Asset::FieldType::Bool2;
                case ReflectionBasicType::Type::Bool3: return EffectLibrary::Asset::FieldType::Bool3;
                case ReflectionBasicType::Type::Bool4: return EffectLibrary::Asset::FieldType::Bool4;

                case ReflectionBasicType::Type::Uint64: return EffectLibrary::Asset::FieldType::Uint64;
                case ReflectionBasicType::Type::Uint64_2: return EffectLibrary::Asset::FieldType::Uint64_2;
                case ReflectionBasicType::Type::Uint64_3: return EffectLibrary::Asset::FieldType::Uint64_3;
                case ReflectionBasicType::Type::Uint64_4: return EffectLibrary::Asset::FieldType::Uint64_4;

                case ReflectionBasicType::Type::Int8: return EffectLibrary::Asset::FieldType::Int8;
                case ReflectionBasicType::Type::Int8_2: return EffectLibrary::Asset::FieldType::Int8_2;
                case ReflectionBasicType::Type::Int8_3: return EffectLibrary::Asset::FieldType::Int8_3;
                case ReflectionBasicType::Type::Int8_4: return EffectLibrary::Asset::FieldType::Int8_4;

                case ReflectionBasicType::Type::Int16: return EffectLibrary::Asset::FieldType::Int16;
                case ReflectionBasicType::Type::Int16_2: return EffectLibrary::Asset::FieldType::Int16_2;
                case ReflectionBasicType::Type::Int16_3: return EffectLibrary::Asset::FieldType::Int16_3;
                case ReflectionBasicType::Type::Int16_4: return EffectLibrary::Asset::FieldType::Int16_4;

                case ReflectionBasicType::Type::Int: return EffectLibrary::Asset::FieldType::Int;
                case ReflectionBasicType::Type::Int2: return EffectLibrary::Asset::FieldType::Int2;
                case ReflectionBasicType::Type::Int3: return EffectLibrary::Asset::FieldType::Int3;
                case ReflectionBasicType::Type::Int4: return EffectLibrary::Asset::FieldType::Int4;

                case ReflectionBasicType::Type::Int64: return EffectLibrary::Asset::FieldType::Int64;
                case ReflectionBasicType::Type::Int64_2: return EffectLibrary::Asset::FieldType::Int64_2;
                case ReflectionBasicType::Type::Int64_3: return EffectLibrary::Asset::FieldType::Int64_3;
                case ReflectionBasicType::Type::Int64_4: return EffectLibrary::Asset::FieldType::Int64_4;

                case ReflectionBasicType::Type::Float16: return EffectLibrary::Asset::FieldType::Float16;
                case ReflectionBasicType::Type::Float16_2: return EffectLibrary::Asset::FieldType::Float16_2;
                case ReflectionBasicType::Type::Float16_3: return EffectLibrary::Asset::FieldType::Float16_3;
                case ReflectionBasicType::Type::Float16_4: return EffectLibrary::Asset::FieldType::Float16_4;

                case ReflectionBasicType::Type::Float16_2x2: return EffectLibrary::Asset::FieldType::Float16_2x2;
                case ReflectionBasicType::Type::Float16_2x3: return EffectLibrary::Asset::FieldType::Float16_2x3;
                case ReflectionBasicType::Type::Float16_2x4: return EffectLibrary::Asset::FieldType::Float16_2x4;
                case ReflectionBasicType::Type::Float16_3x2: return EffectLibrary::Asset::FieldType::Float16_3x2;
                case ReflectionBasicType::Type::Float16_3x3: return EffectLibrary::Asset::FieldType::Float16_3x3;
                case ReflectionBasicType::Type::Float16_3x4: return EffectLibrary::Asset::FieldType::Float16_3x4;

                case ReflectionBasicType::Type::Float: return EffectLibrary::Asset::FieldType::Float;
                case ReflectionBasicType::Type::Float2: return EffectLibrary::Asset::FieldType::Float2;
                case ReflectionBasicType::Type::Float3: return EffectLibrary::Asset::FieldType::Float3;
                case ReflectionBasicType::Type::Float4: return EffectLibrary::Asset::FieldType::Float4;

                case ReflectionBasicType::Type::Float2x2: return EffectLibrary::Asset::FieldType::Float2x2;
                case ReflectionBasicType::Type::Float2x3: return EffectLibrary::Asset::FieldType::Float2x3;
                case ReflectionBasicType::Type::Float2x4: return EffectLibrary::Asset::FieldType::Float2x4;
                case ReflectionBasicType::Type::Float3x2: return EffectLibrary::Asset::FieldType::Float3x2;
                case ReflectionBasicType::Type::Float3x3: return EffectLibrary::Asset::FieldType::Float3x3;
                case ReflectionBasicType::Type::Float3x4: return EffectLibrary::Asset::FieldType::Float3x4;
                case ReflectionBasicType::Type::Float4x2: return EffectLibrary::Asset::FieldType::Float4x2;
                case ReflectionBasicType::Type::Float4x3: return EffectLibrary::Asset::FieldType::Float4x3;
                case ReflectionBasicType::Type::Float4x4: return EffectLibrary::Asset::FieldType::Float4x4;

                case ReflectionBasicType::Type::Float64: return EffectLibrary::Asset::FieldType::Float64;
                case ReflectionBasicType::Type::Float64_2: return EffectLibrary::Asset::FieldType::Float64_2;
                case ReflectionBasicType::Type::Float64_3: return EffectLibrary::Asset::FieldType::Float64_3;
                case ReflectionBasicType::Type::Float64_4: return EffectLibrary::Asset::FieldType::Float64_4;
                default: THROW("Invalid basic type");
            }
        }
    }

    struct ReflectionCtx
    {
        std::string currentPath;
        std::vector<GAPI::ShaderStage> entryPointStages;
        std::vector<slang::IMetadata*> entryPointMetadatas;
        ref<const ParameterBlockReflection> currentParameterBlock;
        EffectSerializer* serializer;
    };

    ReflectionBuilder::ReflectionBuilder(){}
    ReflectionBuilder::~ReflectionBuilder(){}

    GAPI::ShaderStageMask getParameterUsageMask(ReflectionCtx* ctx, ReflectionResourceType::Type type, BindingLocation bindingLocation)
    {
        ASSERT(ctx != nullptr);

        const auto getResourceCategory = [](ReflectionResourceType::Type type) -> SlangParameterCategory {
            switch (type)
            {
                case ReflectionResourceType::Type::ConstantBuffer: return SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER;
                case ReflectionResourceType::Type::Texture: return SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE;
                default: THROW("Invalid type");
            }
            return SLANG_PARAMETER_CATEGORY_NONE;
        };

        UNUSED(type);
        UNUSED(getResourceCategory);

        // Works only for descriptor table slots
        const auto category = SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;

        GAPI::ShaderStageMask usageMask = GAPI::ShaderStageMask::None;
        for (uint32_t i = 0; i < ctx->entryPointMetadatas.size(); i++)
        {
            const auto& entryPointMetadata = ctx->entryPointMetadatas[i];
            const auto entryPointStage = ctx->entryPointStages[i];

            bool isUsed = false;
            entryPointMetadata->isParameterLocationUsed(category, bindingLocation.registerSpace, bindingLocation.registerIndex, isUsed);
            if (isUsed)
                usageMask |= GAPI::GetShaderStageMask(entryPointStage);
        }

        return usageMask;
    }


    GAPI::GpuResourceDimension getResourceDimension(ReflectionResourceType::Type type, ReflectionResourceType::Dimensions dimensions)
    {
        switch (type)
        {
            case ReflectionResourceType::Type::Texture:
            switch (dimensions)
            {
                case Falcor::ReflectionResourceType::Dimensions::Texture1D: return GAPI::GpuResourceDimension::Texture1D;
                case Falcor::ReflectionResourceType::Dimensions::Texture2D: return GAPI::GpuResourceDimension::Texture2D;
                case Falcor::ReflectionResourceType::Dimensions::Texture3D: return GAPI::GpuResourceDimension::Texture3D;
                case Falcor::ReflectionResourceType::Dimensions::TextureCube: return GAPI::GpuResourceDimension::TextureCube;
                case Falcor::ReflectionResourceType::Dimensions::Texture2DMS: return GAPI::GpuResourceDimension::Texture2DMS;
                case Falcor::ReflectionResourceType::Dimensions::Texture2DMSArray: return GAPI::GpuResourceDimension::Texture2DMS;
                default: THROW("Invalid dimension");
            }
            case ReflectionResourceType::Type::StructuredBuffer:
            case ReflectionResourceType::Type::RawBuffer:
            case ReflectionResourceType::Type::TypedBuffer:
            case ReflectionResourceType::Type::ConstantBuffer:
                return GAPI::GpuResourceDimension::Buffer;
            default: THROW("Invalid type");
        }
    }

    ResourceReflection::Type getResourceType(ReflectionResourceType::Type type)
    {
        switch (type)
        {
            case Falcor::ReflectionResourceType::Type::Texture: return ResourceReflection::Type::Texture;
            case Falcor::ReflectionResourceType::Type::StructuredBuffer: return ResourceReflection::Type::StructuredBuffer;
            case Falcor::ReflectionResourceType::Type::RawBuffer: return ResourceReflection::Type::RawBuffer;
            case Falcor::ReflectionResourceType::Type::TypedBuffer: return ResourceReflection::Type::TypedBuffer;
            case Falcor::ReflectionResourceType::Type::Sampler: return ResourceReflection::Type::Sampler;
            case Falcor::ReflectionResourceType::Type::ConstantBuffer: return ResourceReflection::Type::ConstantBuffer;
            case Falcor::ReflectionResourceType::Type::AccelerationStructure: return ResourceReflection::Type::AccelerationStructure;
            default: THROW("Invalid type");
        }
    }

    GAPI::TextureSampleType getTextureSampleType(ReflectionResourceType::ReturnType returnType)
    {
        switch (returnType)
        {
            case ReflectionResourceType::ReturnType::Float: return GAPI::TextureSampleType::Float;
            case ReflectionResourceType::ReturnType::Int: return GAPI::TextureSampleType::Int;
            case ReflectionResourceType::ReturnType::Uint: return GAPI::TextureSampleType::Uint;
            case ReflectionResourceType::ReturnType::Double: return GAPI::TextureSampleType::Double;
            case ReflectionResourceType::ReturnType::Unknown: return GAPI::TextureSampleType::Undefined;
            default: throw std::runtime_error("Invalid return type");
        }
    }

    void ReflectionBuilder::reflect(ReflectionCtx* ctx, const std::shared_ptr<const Falcor::ReflectionVar>& var)
    {
        ASSERT(ctx != nullptr);
        auto type = var->getType();

        if (type->getKind() == ReflectionType::Kind::Resource)
        {
            const auto currentParameterBlock = ctx->currentParameterBlock;
            const auto resourceBinding = currentParameterBlock->getResourceBinding(var->getName());
            const auto bindingInfo = currentParameterBlock->getResourceRangeBindingInfo(resourceBinding.getResourceRangeIndex());
            const auto resourceType = type->asResourceType();

            ResourceReflection resourceReflection;
            resourceReflection.name = ctx->currentPath + var->getName();
            resourceReflection.bindingLocation.registerIndex = bindingInfo.regIndex;
            resourceReflection.bindingLocation.registerSpace = bindingInfo.regSpace;
            resourceReflection.dimension = getResourceDimension(resourceType->getType(), resourceType->getDimensions());
            resourceReflection.type = getResourceType(resourceType->getType());
            resourceReflection.usageMask = getParameterUsageMask(ctx, resourceType->getType(), resourceReflection.bindingLocation);
            resourceReflection.count = 0;
            resourceReflection.sampleType = getTextureSampleType(resourceType->getReturnType());

            type->getSlangTypeLayout()->getElementTypeLayout();

            const auto structType = resourceType->getStructType();
            if (structType != nullptr)
            {
               // ASSERT(resourceType->getType() == ReflectionResourceType::Type::ConstantBuffer);
              //  resourceReflection.structIndex = ctx->structs.size();

                const auto currentPath = ctx->currentPath;
                ctx->currentPath += var->getName() + ".";
                ctx->currentParameterBlock = resourceType->getParameterBlockReflector();

                //StructReflection structReflection;
                for (uint32_t j = 0; j < structType->getMemberCount(); j++)
                    reflect(ctx, structType->getMember(j));

                ctx->currentParameterBlock = currentParameterBlock;
                ctx->currentPath = currentPath;
                // ctx->structs.emplace_back(std::move(structReflection));
            }

          //  ctx->resources.emplace_back(std::move(resourceReflection));
          ctx->serializer->AddResource(std::move(resourceReflection));
        } else
        {
            ASSERT(type->getKind() == ReflectionResourceType::Kind::Basic ||
                   type->getKind() == ReflectionResourceType::Kind::Struct ||
                   type->getKind() == ReflectionResourceType::Kind::Array);

            EffectLibrary::FieldReflection fieldReflection;
            fieldReflection.name = var->getName().c_str();
            fieldReflection.type = getFieldType(type->asBasicType()->getType());
            fieldReflection.kind = EffectLibrary::FieldKind::Basic;
            fieldReflection.structIndex = 0;
            fieldReflection.arraySize = 0;
            fieldReflection.offset = 0;
            fieldReflection.size = 0;
            ctx->serializer->AddField(std::move(fieldReflection));
        }
    }

    void ReflectionBuilder::Build(EffectSerializer* serializer, slang::IComponentType* program, slang::ShaderReflection* reflection)
    {
        ASSERT(serializer != nullptr);
        ASSERT(program != nullptr);
        ASSERT(reflection != nullptr);

        std::cout << "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" << std::endl;

        std::string log;
        auto programVersion = eastl::make_unique<Falcor::ProgramVersion>(reflection->getSession());
        try
        {
            std::vector<slang::EntryPointLayout*> entryPointLayouts;

            for (uint32_t i = 0; i < reflection->getEntryPointCount(); i++)
                entryPointLayouts.emplace_back(reflection->getEntryPointByIndex(i));

            auto programReflection = ProgramReflection::create(programVersion.get(), reflection, entryPointLayouts, log);
            auto defaultParameterBlock = programReflection->getDefaultParameterBlock();

            ReflectionCtx ctx;
            ctx.currentParameterBlock = defaultParameterBlock;
            ctx.serializer = serializer;

            for (uint32_t i = 0; i < reflection->getEntryPointCount(); i++)
            {
                slang::IMetadata* metadata;
                program->getEntryPointMetadata(i, 0, &metadata);
                ctx.entryPointMetadatas.emplace_back(metadata);
                ctx.entryPointStages.emplace_back(GetShaderStage(reflection->getEntryPointByIndex(i)->getStage()));
            }

            bool hasGlobalUniforms = false;
            for (uint32_t i = 0; i < defaultParameterBlock->getResourceCount(); i++)
            {
                auto resource = defaultParameterBlock->getResource(i);

                if (resource->getType()->getKind() == ReflectionType::Kind::Resource)
                {
                    hasGlobalUniforms = true;
                    break;
                }
            }


        /*    StructReflection* globalUniformsStructReflectionPtr = nullptr;
*/
            if (hasGlobalUniforms)
            {
                ResourceReflection globalUniformsConstantBuffer;

                // We can't extratc that information from the reflection, so we need to hardcode it.
                globalUniformsConstantBuffer.name = "uniforms";
                globalUniformsConstantBuffer.bindingLocation.registerIndex = 0;
                globalUniformsConstantBuffer.bindingLocation.registerSpace = 0;
                globalUniformsConstantBuffer.dimension = GAPI::GpuResourceDimension::Buffer;
                globalUniformsConstantBuffer.type = ResourceReflection::Type::ConstantBuffer;
               // globalUniformsConstantBuffer.structIndex = 0;
                globalUniformsConstantBuffer.count = 0;
                globalUniformsConstantBuffer.sampleType = GAPI::TextureSampleType::Undefined;
                globalUniformsConstantBuffer.format = GAPI::GpuResourceFormat::Unknown;
                globalUniformsConstantBuffer.usageMask = getParameterUsageMask(&ctx, ReflectionResourceType::Type::ConstantBuffer, globalUniformsConstantBuffer.bindingLocation);

                ctx.serializer->AddResource(std::move(globalUniformsConstantBuffer));
            }

            for (uint32_t i = 0; i < defaultParameterBlock->getResourceCount(); i++)
            {
                auto resource = defaultParameterBlock->getResource(i);

                reflect(&ctx, resource);
            }

          /*   for (auto& resource : ctx.resources)
            {
                std::cout << "resource: " << resource.name
                          << " binding location: " << resource.bindingLocation.registerIndex << " " << resource.bindingLocation.registerSpace
                   //       << " dimension: " << enumToString(resource.dimension)
                          << " usage mask: " << getUsageMaskString(resource.usageMask)
                     //     << " type: " << enumToString(resource.type) << std::endl
                     << std::endl;

               if (resource.type == ReflectionResourceType::Type::ConstantBuffer && resource.structIndex != ResourceReflection::invalidStructIndex)
                {
                    const auto& structReflection = structs[resource.structIndex];
                    for (const auto& field : structReflection.fields)
                    {
                        std::cout << "field: " << field.name << std::endl;
                    }
                }
            }*/



        }
        catch (const std::exception& e)
        {
            std::cerr << "Error creating program reflection: " << e.what() << std::endl;
            // return Common::RResult::Fail;
        }
    }
}


