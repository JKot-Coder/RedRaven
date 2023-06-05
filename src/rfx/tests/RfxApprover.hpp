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
            static void verify(const std::vector<Common::ComPtr<Rfx::ICompileResult>>& compileResults, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter writer(compileResults);
                ApprovalTests::Approvals::verify(writer, options);
            }

            static void verify2(const raymii::CommandResult& commandResult, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter2 writer(commandResult);
                ApprovalTests::Approvals::verify(writer, options);
            }

            static void verify3(Common::RResult result, const U8String& result2, const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                RfxWriter3 writer(result, result2);
                ApprovalTests::Approvals::verify(writer, options);
            }
        };
    }
}