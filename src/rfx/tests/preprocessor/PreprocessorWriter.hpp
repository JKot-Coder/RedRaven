#pragma once

#include "ApprovalTests/ApprovalTests.hpp"
#include "rfx.hpp"

namespace RR::Rfx
{
        namespace Tests
    {
        class PreprocessorWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            PreprocessorWriter::PreprocessorWriter(const ComPtr<IBlob>& preprocesorOutput, const ComPtr<IBlob>& diagnosticOutput)
                : preprocesorOutput_(preprocesorOutput), diagnosticOutput_(diagnosticOutput)
            {
            }

            std::string getFileExtensionWithDot() const override
            {
                return ".out";
            }

            void write(std::string path) const override;

            void cleanUpReceived(std::string receivedPath) const override
            {
                remove(receivedPath.c_str());
            }

        private:
            ComPtr<IBlob> preprocesorOutput_;
            ComPtr<IBlob> diagnosticOutput_;
        };
    }
}
