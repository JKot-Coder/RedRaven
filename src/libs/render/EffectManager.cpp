#include "EffectManager.hpp"

#include "render/DeviceContext.hpp"
#include "render/Effect.hpp"

#include "effect_library/EffectFormat.hpp"
#include "effect_library/EffectLibrary.hpp"

#include "gapi/BindingGroupLayout.hpp"
#include "gapi/Limits.hpp"

#include "gapi/GpuResource.hpp"
#include <EASTL/fixed_vector.h>

#include "common/Result.hpp"
#include "common/hashing/Hash.hpp"

#include <limits>

namespace RR::Render
{
    EffectManager::EffectManager() { };
    EffectManager::~EffectManager() { };

    Common::RResult EffectManager::Init(std::string_view path)
    {
        ASSERT(!effectLibrary);

        effectLibrary = eastl::make_unique<EffectLibrary::EffectLibrary>();
        RR_RETURN_ON_FAIL(effectLibrary->Load(path));

        auto& deviceContext = DeviceContext::Instance();

        shaders.reserve(effectLibrary->GetShaderCount());
        for (size_t i = 0; i < effectLibrary->GetShaderCount(); i++)
        {
            const EffectLibrary::ShaderDesc& shaderAssetDesc = effectLibrary->GetShader(i);

            GAPI::ShaderDesc shaderDesc;
            shaderDesc.stage = shaderAssetDesc.stage;
            shaderDesc.data = shaderAssetDesc.data;
            shaderDesc.size = shaderAssetDesc.size;

            shaders.emplace_back(deviceContext.CreateShader(shaderDesc, shaderAssetDesc.name));
        }

        bindingGroupLayouts.reserve(effectLibrary->GetBindingGroupCount());
        for (size_t i = 0; i < effectLibrary->GetBindingGroupCount(); i++)
        {
            const auto& group = effectLibrary->GetBindingGroupReflection(i);

            // --- GPU binding layout ---
            eastl::fixed_vector<GAPI::BindingLayoutElement, GAPI::MAX_BINDINGS_PER_GROUP, false> elements;
            for (const auto& res : group.resources)
            {
                GAPI::BindingLayoutElement element;
                element.type = res.type;
                element.binding = res.binding;
                element.count = res.count;
                element.stageMask = res.usageMask;
                element.dimension = res.dimension;
                element.sampleType = res.sampleType;
                element.format = res.format;

                elements.push_back(element);
            }

            const GAPI::BindingGroupLayoutDesc layoutDesc {elements};
            auto& layout = bindingGroupLayouts.emplace_back(
                deviceContext.CreateBindingGroupLayout(layoutDesc, group.name));

            // --- Reflection metadata ---
            eastl::fixed_vector<GAPI::FieldDesc, 16, true> fields;
            eastl::fixed_vector<GAPI::ResourceSlotDesc, GAPI::MAX_BINDINGS_PER_GROUP, false> resources;
            uint32_t uniformBufferSize = 0;
            uint32_t uniformCbvSlot = GAPI::BindingGroupLayout::INVALID_SLOT;
            uint16_t resourceSlotIndex = 0;

            for (const auto& res : group.resources)
            {
                switch (res.type)
                {
                    case GAPI::BindingType::ConstantBuffer:
                    {
                        if (uniformCbvSlot == GAPI::BindingGroupLayout::INVALID_SLOT)
                            uniformCbvSlot = res.binding;

                        for (const auto& uniform : res.uniformFields)
                        {
                            ASSERT(uniform.offset <= std::numeric_limits<uint16_t>::max());
                            ASSERT(uniform.size <= std::numeric_limits<uint16_t>::max());

                            GAPI::FieldDesc field;
                            field.nameHash = Common::Hash(uniform.name);
                            field.offset = static_cast<uint16_t>(uniform.offset);
                            field.size = static_cast<uint16_t>(uniform.size);
                            fields.push_back(field);

                            const uint32_t extent = uniform.offset + uniform.size;
                            if (extent > uniformBufferSize)
                                uniformBufferSize = extent;
                        }
                        break;
                    }
                    case GAPI::BindingType::TextureSRV:
                    case GAPI::BindingType::TextureUAV:
                    case GAPI::BindingType::BufferSRV:
                    case GAPI::BindingType::BufferUAV:
                    case GAPI::BindingType::Sampler:
                    {
                        GAPI::ResourceSlotDesc slot;
                        slot.nameHash = Common::Hash(res.name);
                        slot.binding = static_cast<uint16_t>(res.binding);
                        slot.slotIndex = resourceSlotIndex++;
                        slot.type = res.type;
                        resources.push_back(slot);
                        break;
                    }
                    default:
                        ASSERT_MSG(false, "Unknown binding type");
                        break;
                }
            }

            GAPI::BindingGroupReflectionDesc reflDesc;
            reflDesc.bindingSpace = group.bindingSpace;
            reflDesc.fields = fields;
            reflDesc.resources = resources;
            reflDesc.uniformBufferSize = uniformBufferSize;
            reflDesc.uniformCbvSlot = uniformCbvSlot;
            layout->InitReflection(reflDesc);

            layoutMap[Common::Hash(group.name)] = layout.get();
        }

        return Common::RResult::Ok;
    }

    eastl::unique_ptr<Effect> EffectManager::Load(const std::string& name)
    {
        ASSERT(effectLibrary);

        auto& deviceContext = DeviceContext::Instance();

        EffectLibrary::EffectDesc libraryEffectDesc;

        // TODO, Could be replace with const expr hash. Todo replace with hashed string
        if (!effectLibrary->GetEffectDesc(Common::Hash(name), libraryEffectDesc))
        {
            LOG_ERROR("Failed to get effect desc for effect: {}", name);
            return nullptr;
        }

        Render::EffectDesc effectDesc;
        for (auto& pass : libraryEffectDesc.passes)
        {
            Render::EffectDesc::PassDesc passDesc;
            passDesc.name = pass.name;
            passDesc.rasterizerDesc = pass.rasterizerDesc;
            passDesc.depthStencilDesc = pass.depthStencilDesc;
            passDesc.blendDesc = pass.blendDesc;

            static_assert(eastl::tuple_size<decltype(passDesc.shaders)>::value == eastl::tuple_size<decltype(pass.shaderIndexes)>::value);
            for (uint32_t i = 0; i < pass.shaderIndexes.size(); i++)
            {
                const auto shaderIndex = pass.shaderIndexes[i];
                if (shaderIndex == EffectLibrary::Asset::INVALID_INDEX)
                    continue;

                passDesc.shaders[i] = shaders[shaderIndex].get();
            }

            effectDesc.passes.emplace_back(eastl::move(passDesc));
        }

        auto effect = deviceContext.CreateEffect(name, eastl::move(effectDesc));
        return effect;
    }

}
