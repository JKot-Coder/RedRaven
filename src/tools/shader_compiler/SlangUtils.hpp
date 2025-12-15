#pragma once

enum SlangStage : uint32_t;

namespace RR
{
    namespace GAPI
    {
        enum class ShaderStage : uint8_t;
    }

    GAPI::ShaderStage GetShaderStage(SlangStage stage);
}
