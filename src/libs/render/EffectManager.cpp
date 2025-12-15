#include "EffectManager.hpp"

#include "render/DeviceContext.hpp"
#include "render/Effect.hpp"

#include "effect_library/EffectLibrary.hpp"
#include "effect_library/EffectFormat.hpp"

#include "common/Result.hpp"
#include "common/hashing/Hash.hpp"

namespace RR::Render
{
    EffectManager::EffectManager() {};
    EffectManager::~EffectManager() {};

    Common::RResult EffectManager::Init(std::string_view path)
    {
        ASSERT(!effectLibrary);

        effectLibrary = eastl::make_unique<EffectLibrary::EffectLibrary>();
        RR_RETURN_ON_FAIL(effectLibrary->Load(path));

        return Common::RResult::Ok;
    }

    eastl::unique_ptr<Effect> EffectManager::Load(const std::string& name)
    {
        ASSERT(effectLibrary);

        auto& deviceContext = DeviceContext::Instance();

        EffectLibrary::EffectDesc libraryEffectDesc;

        // TODO, Could be replace with const expr hash. Todo replace with hashed string
        if(!effectLibrary->GetEffectDesc(Common::Hash(name), libraryEffectDesc))
        {
            LOG_ERROR("Failed to get effect desc for effect: {}", name);
            return nullptr;
        }

        for(size_t i = 0; i < effectLibrary->GetShaderCount(); i++)
        {
            GAPI::ShaderDesc shaderDesc;
            EffectLibrary::ShaderDesc shaderAssetDesc = effectLibrary->GetShader(i);

            shaderDesc.stage = shaderAssetDesc.stage;
            shaderDesc.data = shaderAssetDesc.data;
            shaderDesc.size = shaderAssetDesc.size;

            shaders.emplace_back(eastl::move(deviceContext.CreateShader(shaderDesc, shaderAssetDesc.name)));
        }

        Render::EffectDesc effectDesc;
        for(auto& pass : libraryEffectDesc.passes)
        {
            Render::EffectDesc::PassDesc passDesc;
            passDesc.name = pass.name;
            passDesc.rasterizerDesc = pass.rasterizerDesc;
            passDesc.depthStencilDesc = pass.depthStencilDesc;
            passDesc.blendDesc = pass.blendDesc;

            static_assert(eastl::tuple_size<decltype(passDesc.shaders)>::value == eastl::tuple_size<decltype(pass.shaderIndexes)>::value);
            for(uint32_t i = 0; i < pass.shaderIndexes.size(); i++)
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