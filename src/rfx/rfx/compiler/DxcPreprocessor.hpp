#pragma once

#include "common/ComPtr.hpp"

namespace RR
{
    namespace Rfx
    {
        using RR::Common::ComPtr;
        class SourceFile;

        class DxcPreprocessor
        {
        public:
            DxcPreprocessor() = default;
            RfxResult Preprocess(std::shared_ptr<RR::Rfx::SourceFile>& source, ComPtr<IBlob>& output, ComPtr<IBlob>& diagnostic);

            void DefineMacro(const std::string& define) { defines.push_back(define); }

        private:
            std::vector<std::string> defines;
        };
    }
}