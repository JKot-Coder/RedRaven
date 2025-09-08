#pragma once

#include "gapi/PipelineState.hpp"

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::EffectAsset
{
    class EffectLibrary
    {
    public:
        EffectLibrary() = default;
        ~EffectLibrary() = default;

        Common::RResult Load(const std::string& path);
    private:
        std::unique_ptr<std::byte[]> stringsData;
        eastl::vector<const char*> strings;
    };
}