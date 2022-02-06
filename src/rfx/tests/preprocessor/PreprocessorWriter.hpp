#pragma once

#include "compiler/DiagnosticSink.hpp"

#include "ApprovalTests/ApprovalTests.hpp"

namespace RR::Rfx
{
    class Preprocessor;

    namespace Tests
    {
        class PreprocessorWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            explicit PreprocessorWriter(const std::shared_ptr<Preprocessor>& preprocessor, const std::shared_ptr<BufferWriter>& disagnosticBuffer)
                : preprocessor_(preprocessor), disagnosticBuffer_(disagnosticBuffer)
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
            std::shared_ptr<Preprocessor> preprocessor_;
            std::shared_ptr<BufferWriter> disagnosticBuffer_;
        };
    }
}
