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
            static void verify(const ComPtr<IBlob>& preprocessorOutput, const ComPtr<IBlob>& diagnosticOutput, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter writer(preprocessorOutput, diagnosticOutput);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}