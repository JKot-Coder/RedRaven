#pragma once

#include "preprocessor/PreprocessorWriter.hpp"

#include "rfx.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    class Preprocessor;

    namespace Tests
    {
        class PreprocessorApprover
        {
        public:
            static void verify(const ComPtr<IBlob>& preprocessorOutput, const ComPtr<IBlob>& diagnosticOutput, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                PreprocessorWriter writer(preprocessorOutput, diagnosticOutput);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}