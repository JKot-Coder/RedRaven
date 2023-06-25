#pragma once

#include "RfxWriter.hpp"

#include "rfx.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace raymii
{
    struct CommandResult;
}

namespace RR::Rfx
{
    namespace Tests
    {
        class RfxApprover
        {
        public:
            static void verify(Common::ComPtr<Rfx::ICompileResult>& compileResult, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter writer(compileResult);
                ApprovalTests::Approvals::verify(writer, options);
            }

            static void verify(const raymii::CommandResult& commandResult, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter2 writer(commandResult);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}