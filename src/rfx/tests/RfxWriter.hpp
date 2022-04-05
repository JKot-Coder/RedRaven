#pragma once

#include "ApprovalTests/ApprovalTests.hpp"
#include "rfx.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class RfxWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            ~RfxWriter() { }
            RfxWriter::RfxWriter(const std::vector<ComPtr<Rfx::ICompileResult>>& compileResults)
                : compileResults_(compileResults)
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
            std::vector<ComPtr<Rfx::ICompileResult>> compileResults_;
        };
    }
}
