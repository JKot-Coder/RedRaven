#pragma once

namespace slang
{
    class ShaderReflection;
    struct IComponentType;
}

namespace Falcor
{
    class ReflectionVar;
}

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    struct ReflectionCtx;
    class EffectSerializer;
    struct ResourceReflection;
    struct UniformDesc;

    class ReflectionBuilder
    {
    public:
        ReflectionBuilder();
        ~ReflectionBuilder();

        Common::RResult Build(EffectSerializer& serializer, slang::IComponentType* program, slang::ShaderReflection* reflection, uint32_t& globalBindingGroupIndex);

    private:
        eastl::unique_ptr<ReflectionCtx> ctx;
    };
}