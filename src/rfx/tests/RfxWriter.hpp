#pragma once

#include "ApprovalTests/ApprovalTests.hpp"
#include "command.h"
#include "rfx.hpp"

#include "common/ComPtr.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        class RfxWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            ~RfxWriter() { }
            RfxWriter(Common::ComPtr<Rfx::ICompileResult>& compileResult)
                : compileResult_(compileResult)
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
            Common::ComPtr<Rfx::ICompileResult> compileResult_;
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