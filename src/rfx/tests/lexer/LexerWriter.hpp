#pragma once

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        /* class LexerWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            explicit LexerWriter(const std::shared_ptr<Lexer>& lexer, const std::shared_ptr<BufferWriter>& disagnosticBuffer)
                : lexer_(lexer), disagnosticBuffer_(disagnosticBuffer)
            {
            }

            std::string getFileExtensionWithDot() const override
            {
                return ".json";
            }

            void write(std::string path) const override;

            void cleanUpReceived(std::string receivedPath) const override
            {
                remove(receivedPath.c_str());
            }

        private:
            std::shared_ptr<Lexer> lexer_;
            std::shared_ptr<BufferWriter> disagnosticBuffer_;
        };*/
    }
}
