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
    struct ReflectionCtx;
    class EffectSerializer;

    class ReflectionBuilder
    {
    public:
        ReflectionBuilder();
        ~ReflectionBuilder();

        void Build(EffectSerializer* serializer, slang::IComponentType* program, slang::ShaderReflection* reflection);

    private:
        void reflect(ReflectionCtx* ctx, const std::shared_ptr<const Falcor::ReflectionVar>& var);
    };
}