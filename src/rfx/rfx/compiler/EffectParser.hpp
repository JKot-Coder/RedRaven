#pragma once

#include <filesystem>

namespace RR::Common
{
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