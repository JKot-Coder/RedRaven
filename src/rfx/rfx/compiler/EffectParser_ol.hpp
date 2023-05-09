#pragma once

#include <filesystem>

namespace RR::Common
{
    enum class RfxResult : uint32_t;
}

namespace RR::Rfx
{
    class EffectParser
    {
        public:
            RfxResult Parse(const std::filesystem::path& path);
    };
}