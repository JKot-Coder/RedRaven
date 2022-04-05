#pragma once

#include "RfxWriter.hpp"

#include "rfx.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class RfxApprover
        {
        public:
            static void verify(const std::vector<ComPtr<Rfx::ICompileResult>>& compileResults, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter writer(compileResults);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}