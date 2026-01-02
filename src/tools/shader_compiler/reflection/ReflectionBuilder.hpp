#pragma once

namespace slang
{
    class ShaderReflection;
}

namespace RR
{
    class ReflectionBuilder
    {
    public:
        ReflectionBuilder() = default;

        void Build(slang::ShaderReflection* reflection);
    };
}