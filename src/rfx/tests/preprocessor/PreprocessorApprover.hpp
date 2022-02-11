#pragma once

#include "compiler/DiagnosticSink.hpp"
#include "tests/preprocessor/PreprocessorWriter.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    class Preprocessor;

    namespace Tests
    {
        class PreprocessorApprover
        {
        public:
            static void verify(const std::shared_ptr<Preprocessor>& preprocessor, const std::shared_ptr<BufferWriter>& disagnosticLogBuffer,
                               const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                PreprocessorWriter writer(preprocessor, disagnosticLogBuffer);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}