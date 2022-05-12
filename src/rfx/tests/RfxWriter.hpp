#pragma once

#include "ApprovalTests/ApprovalTests.hpp"
#include "rfx.hpp"
#include "command.h"

namespace RR::Rfx
{
    namespace Tests
    {
        class RfxWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            ~RfxWriter() { }
            RfxWriter(const std::vector<ComPtr<Rfx::ICompileResult>>& compileResults)
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

        class RfxWriter2 : public ApprovalTests::ApprovalWriter
        {
        public:
            ~RfxWriter2() = default;

            RfxWriter2(const raymii::CommandResult& commandResult)
                : commandResult_(commandResult)
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
            raymii::CommandResult commandResult_;
        };
    }
}