#pragma once

#include "slang-com-ptr.h"

#include <string>

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    class SpirvToWgslTranscoder
    {
    public:
        static Common::RResult Transcode(slang::IBlob* spirvCode, std::string& wgslCode);

    private:
        SpirvToWgslTranscoder() = delete;
    };
}

