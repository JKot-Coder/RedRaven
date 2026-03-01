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
#include "effect_library/EffectFormat.hpp"

#define THROW(message)                     \
    {                                      \
        ASSERT_MSG(false, message);        \
        throw std::runtime_error(message); \
    }                                      \
    (void)0

namespace RR
{
    namespace
    {
        struct BindingLocation
        {
            uint32_t registerIndex;
            uint32_t registerSpace;
        };

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

        ResourceReflection::Access getShaderAccess(ReflectionResourceType::ShaderAccess access)
        {
            switch (access)
            {
                case ReflectionResourceType::ShaderAccess::Read: return ResourceReflection::Access::Read;
                case ReflectionResourceType::ShaderAccess::ReadWrite: return ResourceReflection::Access::ReadWrite;
                default: throw std::runtime_error("Invalid access");
            }
        }
    }

    struct ReflectionCtx
    {
        ReflectionCtx(EffectSerializer& serializer)
            : serializer(&serializer) { }

        void BeginStruct(const std::string& name)
        {
            std::cout << "begin struct" << std::endl;
            Layout layout;
            layout.name = name;
            layout.type = BlockType::Struct;
            stack.push_back(layout);
        }

        void EndStruct()
        {
            if (stack.empty())
                THROW("Struct is not opened");

            auto& currentLayout = stack.back();

            if (currentLayout.type != BlockType::Struct)
                THROW("Expected struct block, but got different block type");

            const auto& name = currentLayout.name;

            std::cout << "end struct: " << name << std::endl;

            UniformDesc structDesc;
            structDesc.name = name.c_str();
            structDesc.type = RR::EffectLibrary::Asset::FieldType::Struct;
            structDesc.arraySize = 0; //??
            structDesc.offset = 0; //??
            structDesc.size = 0; //??
            structDesc.layoutIndex = serializer->AddLayout(currentLayout.uniforms);

            stack.pop_back();
            serializer->AddUniform(structDesc);
        }

        void BeginBindGroup(const std::string& name, const ref<const ParameterBlockReflection>& parameterBlock)
        {
            std::cout << "begin bind group" << std::endl;
            Layout layout;
            layout.name = name;
            layout.type = BlockType::BindGroup;
            layout.rootParameterBlock = currentParameterBlock;
            stack.push_back(layout);
            currentParameterBlock = parameterBlock;
        }

        uint32_t EndBindGroup()
        {
            if (stack.empty())
                THROW("Bind group is not opened");

            auto& currentLayout = stack.back();

            if (currentLayout.type != BlockType::BindGroup)
                THROW("Expected bind group block, but got different block type");

            const auto rootParameterBlock = currentLayout.rootParameterBlock;
            const auto& name = currentLayout.name;
            BindingLocation bindingLocation {0, 0};

            if (rootParameterBlock)
            {
                const auto resourceBinding = rootParameterBlock->getResourceBinding(name);
                const auto bindingInfo = rootParameterBlock->getResourceRangeBindingInfo(resourceBinding.getResourceRangeIndex());
                bindingLocation.registerIndex = bindingInfo.regIndex;
                bindingLocation.registerSpace = bindingInfo.regSpace;
            }

            uint32_t uniformCBVIndex = EffectLibrary::Asset::INVALID_INDEX;
            if (currentParameterBlock->hasDefaultConstantBuffer())
                uniformCBVIndex = currentParameterBlock->getDefaultConstantBufferBindingInfo().regIndex;

            BindGroupDesc bindGroupDesc;
            bindGroupDesc.name = name;
            bindGroupDesc.bindingSpace = bindingLocation.registerSpace;

            if (uniformCBVIndex != RR::EffectLibrary::Asset::INVALID_INDEX)
            {
                ResourceReflection resourceReflection;
                resourceReflection.name = "defaultConstantBuffer";
                resourceReflection.bindingIndex = uniformCBVIndex;
                resourceReflection.type = ResourceReflection::Type::ConstantBuffer;
                resourceReflection.usageMask = GetParameterUsageMask(ReflectionResourceType::Type::ConstantBuffer, BindingLocation {bindingLocation.registerIndex, bindingLocation.registerSpace});
                resourceReflection.dimension = GAPI::GpuResourceDimension::Buffer;
                resourceReflection.format = GAPI::GpuResourceFormat::Unknown;
                resourceReflection.count = 0;
                resourceReflection.layoutIndex = serializer->AddLayout(currentLayout.uniforms);

                bindGroupDesc.uniformCBV = serializer->AddResource(resourceReflection);
                currentLayout.resources.push_back(bindGroupDesc.uniformCBV);
            }

            bindGroupDesc.resourcesLayoutIndex = serializer->AddLayout(currentLayout.resources);
            bindGroupDesc.childsLayoutIndex = serializer->AddLayout(currentLayout.childBindGroups);

            std::cout << "end bind group: " << name
                      << " binding space: " << bindingLocation.registerSpace
                      << std::endl;

            const uint32_t bindGroupIndex = serializer->AddBindGroup(bindGroupDesc);

            currentParameterBlock = currentLayout.rootParameterBlock;
            stack.pop_back();

            if (!stack.empty())
            {
                if (stack.back().type != BlockType::BindGroup)
                    THROW("Bind group can only be added to bind group");

                stack.back().childBindGroups.push_back(bindGroupIndex);
            }

            return bindGroupIndex;
        }

        void BeginConstantBuffer(const std::string& name, const ReflectionResourceType* resourceType)
        {
            std::cout << "begin constant buffer" << std::endl;
            Layout layout;
            layout.name = name;
            layout.type = BlockType::ConstantBuffer;
            layout.resourceType = resourceType;
            layout.rootParameterBlock = stack.back().rootParameterBlock;
            stack.push_back(layout);
        }

        void EndConstantBuffer()
        {
            if (stack.empty())
                THROW("Constant buffer is not opened");

            auto& currentLayout = stack.back();

            if (currentLayout.type != BlockType::ConstantBuffer)
                THROW("Expected constant buffer block, but got different block type");

            const auto& name = currentLayout.name;
            const auto& resourceType = currentLayout.resourceType;

            if (!resourceType)
                THROW("ResourceType is not set for constant buffer");

            std::cout << "end constant buffer: " << name << std::endl;

            const auto resourceBinding = currentParameterBlock->getResourceBinding(name);
            const auto bindingInfo = currentParameterBlock->getResourceRangeBindingInfo(resourceBinding.getResourceRangeIndex());

            // Для constant buffer создаем ResourceReflection напрямую (не через AddResource)
            ResourceReflection resourceReflection;
            resourceReflection.name = name;
            resourceReflection.bindingIndex = bindingInfo.regIndex;
            resourceReflection.dimension = getResourceDimension(resourceType->getType(), resourceType->getDimensions());
            resourceReflection.type = getResourceType(resourceType->getType());
            resourceReflection.usageMask = GetParameterUsageMask(resourceType->getType(), BindingLocation {bindingInfo.regIndex, bindingInfo.regSpace});
            resourceReflection.count = 0;
            resourceReflection.sampleType = getTextureSampleType(resourceType->getReturnType());
            resourceReflection.layoutIndex = serializer->AddLayout(currentLayout.uniforms);

            const auto index = serializer->AddResource(resourceReflection);
            currentLayout.resources.push_back(index);
            stack.pop_back();
        }

    public:
        void AddUniform(const UniformDesc& uniform)
        {
            if (stack.empty())
                THROW("Uniform is not in a block");

            auto& currentLayout = stack.back();

            const auto index = serializer->AddUniform(uniform);
            currentLayout.uniforms.push_back(index);
        }

        uint32_t AddResource(const std::string& name, const ReflectionResourceType* resourceType)
        {
            if (stack.empty())
                THROW("Resource is not in a block");

            auto& currentLayout = stack.back();

            if (currentLayout.type != BlockType::BindGroup)
                THROW("Resource can only be added to bind group");

            if (!resourceType)
                THROW("ResourceType is not set");

            const auto resourceBinding = currentParameterBlock->getResourceBinding(name);
            const auto bindingInfo = currentParameterBlock->getResourceRangeBindingInfo(resourceBinding.getResourceRangeIndex());

            if (bindingInfo.flavor == ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ConstantBuffer ||
                bindingInfo.flavor == ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ParameterBlock)
            {
                THROW("AddResource can only be used for simple resources, not constant buffers or parameter blocks");
            }

            ResourceReflection resourceReflection;
            resourceReflection.name = name;
            resourceReflection.bindingIndex = bindingInfo.regIndex;
            resourceReflection.dimension = getResourceDimension(resourceType->getType(), resourceType->getDimensions());
            resourceReflection.type = getResourceType(resourceType->getType());
            resourceReflection.usageMask = GetParameterUsageMask(resourceType->getType(), BindingLocation {bindingInfo.regIndex, bindingInfo.regSpace});
            resourceReflection.count = 0;
            resourceReflection.sampleType = getTextureSampleType(resourceType->getReturnType());
            resourceReflection.access = getShaderAccess(resourceType->getShaderAccess()); // TODO to support write only acess need parse WGSL;
            resourceReflection.format = {}; // TODO need parse wgsl output for UAV;

            const auto index = serializer->AddResource(resourceReflection);

            currentLayout.resources.push_back(index);
            return index;
        }

        GAPI::ShaderStageMask GetParameterUsageMask(ReflectionResourceType::Type type, BindingLocation bindingLocation) const
        {
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
            for (uint32_t i = 0; i < entryPointMetadatas.size(); i++)
            {
                const auto& entryPointMetadata = entryPointMetadatas[i];
                const auto entryPointStage = entryPointStages[i];

                bool isUsed = false;
                entryPointMetadata->isParameterLocationUsed(category, bindingLocation.registerSpace, bindingLocation.registerIndex, isUsed);
                if (isUsed)
                    usageMask |= GAPI::GetShaderStageMask(entryPointStage);
            }

            return usageMask;
        }

        ref<const ParameterBlockReflection> GetCurrentParameterBlock() const
        {
            return currentParameterBlock;
        }

        void Reflect(const std::shared_ptr<const Falcor::ReflectionVar>& var)
        {
            auto type = var->getType();

            switch (type->getKind())
            {
                case ReflectionType::Kind::Resource:
                {
                    const auto resourceType = type->asResourceType();
                    const auto structType = resourceType->getStructType();

                    if (structType != nullptr)
                    {
                        const auto resourceBinding = currentParameterBlock->getResourceBinding(var->getName());
                        const auto bindingInfo = currentParameterBlock->getResourceRangeBindingInfo(resourceBinding.getResourceRangeIndex());

                        if (bindingInfo.flavor == ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ParameterBlock)
                        {
                            BeginBindGroup(var->getName(), resourceType->getParameterBlockReflector());
                            for (uint32_t j = 0; j < structType->getMemberCount(); j++)
                                Reflect(structType->getMember(j));
                            EndBindGroup();
                        }
                        else if (bindingInfo.flavor == ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ConstantBuffer)
                        {
                            BeginConstantBuffer(var->getName(), resourceType);
                            for (uint32_t j = 0; j < structType->getMemberCount(); j++)
                                Reflect(structType->getMember(j));
                            EndConstantBuffer();
                        }
                        else
                        {
                            THROW("Invalid flavor for resource with struct type");
                        }
                    }
                    else
                    {
                        AddResource(var->getName(), resourceType);
                    }

                    break;
                }
                case ReflectionType::Kind::Basic:
                {
                    const auto basicType = type->asBasicType();

                    UniformDesc uniformDesc;
                    uniformDesc.name = var->getName().c_str();
                    uniformDesc.type = getFieldType(basicType->getType());
                    uniformDesc.arraySize = 0;
                    uniformDesc.offset = var->getByteOffset();
                    uniformDesc.size = basicType->getByteSize();
                    AddUniform(std::move(uniformDesc));

                    break;
                }
                case ReflectionType::Kind::Struct:
                {
                    const auto structType = type->asStructType();

                    UniformDesc uniformDesc;
                    uniformDesc.name = var->getName().c_str();
                    uniformDesc.type = EffectLibrary::Asset::FieldType::Struct;
                    uniformDesc.arraySize = 0;
                    uniformDesc.offset = var->getByteOffset();
                    uniformDesc.size = structType->getByteSize();
                    AddUniform(std::move(uniformDesc));
                    break;
                }
                case ReflectionType::Kind::Array:
                {
                    // TODO: Implement
                    break;
                }
                default:
                    THROW("Invalid type");
            }
        }

    private:
        enum class BlockType
        {
            Struct,
            ConstantBuffer,
            BindGroup
        };

        struct Layout
        {
            eastl::fixed_vector<uint32_t, 16, true> resources;
            eastl::fixed_vector<uint32_t, 16, true> uniforms;
            eastl::fixed_vector<uint32_t, 16, true> childBindGroups;
            std::string name;
            BlockType type;
            const ReflectionResourceType* resourceType = nullptr;
            ref<const ParameterBlockReflection> rootParameterBlock;
        };

        EffectSerializer* serializer;
        std::vector<Layout> stack;
        ref<const ParameterBlockReflection> currentParameterBlock;

    public:
        std::vector<GAPI::ShaderStage> entryPointStages;
        std::vector<slang::IMetadata*> entryPointMetadatas;
    };

    ReflectionBuilder::ReflectionBuilder() { }
    ReflectionBuilder::~ReflectionBuilder() { }

    Common::RResult ReflectionBuilder::Build(EffectSerializer& serializer, slang::IComponentType* program, slang::ShaderReflection* reflection, uint32_t& globalBindingGroupIndex)
    {
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

            ctx = eastl::make_unique<ReflectionCtx>(serializer);

            for (uint32_t i = 0; i < reflection->getEntryPointCount(); i++)
            {
                slang::IMetadata* metadata;
                program->getEntryPointMetadata(i, 0, &metadata);
                ctx->entryPointMetadatas.emplace_back(metadata);
                ctx->entryPointStages.emplace_back(GetShaderStage(reflection->getEntryPointByIndex(i)->getStage()));
            }

            {
                ctx->BeginBindGroup("GlobalUniforms", defaultParameterBlock);
                for (uint32_t i = 0; i < defaultParameterBlock->getResourceCount(); i++)
                {
                    auto resource = defaultParameterBlock->getResource(i);
                    ctx->Reflect(resource);
                }

                globalBindingGroupIndex = ctx->EndBindGroup();
            }

            return Common::RResult::Ok;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error creating program reflection: " << e.what() << std::endl;
            return Common::RResult::Fail;
        }
    }
}
