#pragma once

#include "common/Singleton.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace RR::EffectLibrary
{
    class EffectLibrary;
}

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::Render
{
    class Effect;

    class EffectManager : public Common::Singleton<EffectManager>
    {
    public:
        EffectManager();
        ~EffectManager();

        Common::RResult Init(std::string_view path);
        eastl::unique_ptr<Effect> Load(const std::string& name);

        size_t GetShaderCount() const { return shaders.size(); }

    private:
        eastl::unique_ptr<EffectLibrary::EffectLibrary> effectLibrary;
        eastl::vector<eastl::unique_ptr<GAPI::Shader>> shaders;
        eastl::vector<eastl::unique_ptr<GAPI::BindingGroupLayout>> bindingGroupLayouts;
    };
}