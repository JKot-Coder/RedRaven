#pragma once

#include <filesystem>

namespace RR::Common
{
    enum class RfxResult : uint32_t;
    template <typename T>
    class ComPtr;
}

namespace RR::Rfx
{
    class SourceFile;

    class EffectParser
    {
    public:
        RfxResult Parse(std::shared_ptr<RR::Rfx::SourceFile>& source, Common::ComPtr<IBlob>& output, Common::ComPtr<IBlob>& diagnostic);
    };
}