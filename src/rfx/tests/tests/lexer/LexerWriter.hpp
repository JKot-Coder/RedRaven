#pragma once

#include "compiler/Lexer.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class LexerWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            explicit LexerWriter(const std::shared_ptr<Lexer>& lexer)
                : lexer_(lexer)
            {
            }

            std::string getFileExtensionWithDot() const override
            {
                return ".tokens";
            }

            void write(std::string path) const override;

            void cleanUpReceived(std::string receivedPath) const override
            {
                remove(receivedPath.c_str());
            }

        private:
            std::shared_ptr<Lexer> lexer_;
        };
    }
}
