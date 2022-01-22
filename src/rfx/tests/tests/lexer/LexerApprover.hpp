#pragma once

#include "compiler/Lexer.hpp"
#include "tests/tests/lexer/LexerWriter.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class LexerApprover
        {
        public:
            static void verify(const std::shared_ptr<Lexer>& lexer,
                               const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                LexerWriter lexerWriter(lexer);
                ApprovalTests::Approvals::verify(lexerWriter, options);
            }
        };
    }
}