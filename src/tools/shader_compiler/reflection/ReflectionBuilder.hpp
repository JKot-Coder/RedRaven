#pragma once

#include "effect_library/ShaderReflectionData.hpp"

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