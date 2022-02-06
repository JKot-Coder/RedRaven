#pragma once

#include "compiler/DiagnosticSink.hpp"
#include "compiler/Lexer.hpp"
#include "tests/lexer/LexerWriter.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class LexerApprover
        {
        public:
            static void verify(const std::shared_ptr<Lexer>& lexer, const std::shared_ptr<BufferWriter>& disagnosticLogBuffer,
                               const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                LexerWriter lexerWriter(lexer, disagnosticLogBuffer);
                ApprovalTests::Approvals::verify(lexerWriter, options);
            }
        };
    }
}