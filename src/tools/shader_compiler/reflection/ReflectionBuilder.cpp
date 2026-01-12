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

#include "EffectSerializer.hpp"

#define THROW(message)          \
    ASSERT_MSG(false, message); \
    throw std::runtime_error(message);

namespace RR
{

    struct StructReflection
    {
        struct Field
        {
            enum class Kind
            {
                Array,
                Struct,
                Basic
            };

            enum class BasicType
            {
                Bool,
                Bool2,
                Bool3,
                Bool4,

                Uint8,
                Uint8_2,
                Uint8_3,
                Uint8_4,

                Uint16,
                Uint16_2,
                Uint16_3,
                Uint16_4,

                Uint,
                Uint2,
                Uint3,
                Uint4,

                Uint64,
                Uint64_2,
                Uint64_3,
                Uint64_4,

                Int8,
                Int8_2,
                Int8_3,
                Int8_4,

                Int16,
                Int16_2,
                Int16_3,
                Int16_4,

                Int,
                Int2,
                Int3,
                Int4,

                Int64,
                Int64_2,
                Int64_3,
                Int64_4,

                Float16,
                Float16_2,
                Float16_3,
                Float16_4,

                Float16_2x2,
                Float16_2x3,
                Float16_2x4,
                Float16_3x2,
                Float16_3x3,
                Float16_3x4,
                Float16_4x2,
                Float16_4x3,
                Float16_4x4,

                Float,
                Float2,
                Float3,
                Float4,

                Float2x2,
                Float2x3,
                Float2x4,
                Float3x2,
                Float3x3,
                Float3x4,
                Float4x2,
                Float4x3,
                Float4x4,

                Float64,
                Float64_2,
                Float64_3,
                Float64_4,
            };

            std::string name;
            Kind kind;

            union
            {
                uint32_t arraySize;
                BasicType basicType;
            };
        };

        std::vector<Field> fields;
    };

    namespace
    {
        StructReflection::Field::BasicType getBasicType(ReflectionBasicType::Type type)
        {
            switch (type)
            {
                case ReflectionBasicType::Type::Bool: return StructReflection::Field::BasicType::Bool;
                case ReflectionBasicType::Type::Bool2: return StructReflection::Field::BasicType::Bool2;
                case ReflectionBasicType::Type::Bool3: return StructReflection::Field::BasicType::Bool3;
                case ReflectionBasicType::Type::Bool4: return StructReflection::Field::BasicType::Bool4;

                case ReflectionBasicType::Type::Uint8: return StructReflection::Field::BasicType::Uint8;
                case ReflectionBasicType::Type::Uint8_2: return StructReflection::Field::BasicType::Uint8_2;
                case ReflectionBasicType::Type::Uint8_3: return StructReflection::Field::BasicType::Uint8_3;
                case ReflectionBasicType::Type::Uint8_4: return StructReflection::Field::BasicType::Uint8_4;

                case ReflectionBasicType::Type::Uint16: return StructReflection::Field::BasicType::Uint16;
                case ReflectionBasicType::Type::Uint16_2: return StructReflection::Field::BasicType::Uint16_2;
                case ReflectionBasicType::Type::Uint16_3: return StructReflection::Field::BasicType::Uint16_3;
                case ReflectionBasicType::Type::Uint16_4: return StructReflection::Field::BasicType::Uint16_4;

                case ReflectionBasicType::Type::Uint: return StructReflection::Field::BasicType::Uint;
                case ReflectionBasicType::Type::Uint2: return StructReflection::Field::BasicType::Uint2;
                case ReflectionBasicType::Type::Uint3: return StructReflection::Field::BasicType::Uint3;
                case ReflectionBasicType::Type::Uint4: return StructReflection::Field::BasicType::Uint4;

                case ReflectionBasicType::Type::Uint64: return StructReflection::Field::BasicType::Uint64;
                case ReflectionBasicType::Type::Uint64_2: return StructReflection::Field::BasicType::Uint64_2;
                case ReflectionBasicType::Type::Uint64_3: return StructReflection::Field::BasicType::Uint64_3;
                case ReflectionBasicType::Type::Uint64_4: return StructReflection::Field::BasicType::Uint64_4;

                case ReflectionBasicType::Type::Int8: return StructReflection::Field::BasicType::Int8;
                case ReflectionBasicType::Type::Int8_2: return StructReflection::Field::BasicType::Int8_2;
                case ReflectionBasicType::Type::Int8_3: return StructReflection::Field::BasicType::Int8_3;
                case ReflectionBasicType::Type::Int8_4: return StructReflection::Field::BasicType::Int8_4;

                case ReflectionBasicType::Type::Int16: return StructReflection::Field::BasicType::Int16;
                case ReflectionBasicType::Type::Int16_2: return StructReflection::Field::BasicType::Int16_2;
                case ReflectionBasicType::Type::Int16_3: return StructReflection::Field::BasicType::Int16_3;
                case ReflectionBasicType::Type::Int16_4: return StructReflection::Field::BasicType::Int16_4;

                case ReflectionBasicType::Type::Int: return StructReflection::Field::BasicType::Int;
                case ReflectionBasicType::Type::Int2: return StructReflection::Field::BasicType::Int2;
                case ReflectionBasicType::Type::Int3: return StructReflection::Field::BasicType::Int3;
                case ReflectionBasicType::Type::Int4: return StructReflection::Field::BasicType::Int4;

                case ReflectionBasicType::Type::Int64: return StructReflection::Field::BasicType::Int64;
                case ReflectionBasicType::Type::Int64_2: return StructReflection::Field::BasicType::Int64_2;
                case ReflectionBasicType::Type::Int64_3: return StructReflection::Field::BasicType::Int64_3;
                case ReflectionBasicType::Type::Int64_4: return StructReflection::Field::BasicType::Int64_4;

                case ReflectionBasicType::Type::Float16: return StructReflection::Field::BasicType::Float16;
                case ReflectionBasicType::Type::Float16_2: return StructReflection::Field::BasicType::Float16_2;
                case ReflectionBasicType::Type::Float16_3: return StructReflection::Field::BasicType::Float16_3;
                case ReflectionBasicType::Type::Float16_4: return StructReflection::Field::BasicType::Float16_4;

                case ReflectionBasicType::Type::Float16_2x2: return StructReflection::Field::BasicType::Float16_2x2;
                case ReflectionBasicType::Type::Float16_2x3: return StructReflection::Field::BasicType::Float16_2x3;
                case ReflectionBasicType::Type::Float16_2x4: return StructReflection::Field::BasicType::Float16_2x4;
                case ReflectionBasicType::Type::Float16_3x2: return StructReflection::Field::BasicType::Float16_3x2;
                case ReflectionBasicType::Type::Float16_3x3: return StructReflection::Field::BasicType::Float16_3x3;
                case ReflectionBasicType::Type::Float16_3x4: return StructReflection::Field::BasicType::Float16_3x4;

                case ReflectionBasicType::Type::Float: return StructReflection::Field::BasicType::Float;
                case ReflectionBasicType::Type::Float2: return StructReflection::Field::BasicType::Float2;
                case ReflectionBasicType::Type::Float3: return StructReflection::Field::BasicType::Float3;
                case ReflectionBasicType::Type::Float4: return StructReflection::Field::BasicType::Float4;

                case ReflectionBasicType::Type::Float2x2: return StructReflection::Field::BasicType::Float2x2;
                case ReflectionBasicType::Type::Float2x3: return StructReflection::Field::BasicType::Float2x3;
                case ReflectionBasicType::Type::Float2x4: return StructReflection::Field::BasicType::Float2x4;
                case ReflectionBasicType::Type::Float3x2: return StructReflection::Field::BasicType::Float3x2;
                case ReflectionBasicType::Type::Float3x3: return StructReflection::Field::BasicType::Float3x3;
                case ReflectionBasicType::Type::Float3x4: return StructReflection::Field::BasicType::Float3x4;
                case ReflectionBasicType::Type::Float4x2: return StructReflection::Field::BasicType::Float4x2;
                case ReflectionBasicType::Type::Float4x3: return StructReflection::Field::BasicType::Float4x3;
                case ReflectionBasicType::Type::Float4x4: return StructReflection::Field::BasicType::Float4x4;

                case ReflectionBasicType::Type::Float64: return StructReflection::Field::BasicType::Float64;
                case ReflectionBasicType::Type::Float64_2: return StructReflection::Field::BasicType::Float64_2;
                case ReflectionBasicType::Type::Float64_3: return StructReflection::Field::BasicType::Float64_3;
                case ReflectionBasicType::Type::Float64_4: return StructReflection::Field::BasicType::Float64_4;
                default: THROW("Invalid basic type");
            }
        }
    }

    struct ReflectionCtx
    {
        std::string currentPath;
      //  std::vector<ResourceReflection> resources;
        std::vector<StructReflection> structs;
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
                ASSERT(resourceType->getType() == ReflectionResourceType::Type::ConstantBuffer);
              //  resourceReflection.structIndex = ctx->structs.size();

                const auto currentPath = ctx->currentPath;
                ctx->currentPath += var->getName() + ".";
                ctx->currentParameterBlock = resourceType->getParameterBlockReflector();

                StructReflection structReflection;
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
            std::cout << "reflecting var: " << var->getName() << " is not a resource" << std::endl;
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

            UNUSED(hasGlobalUniforms);
            UNUSED(getBasicType);

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


