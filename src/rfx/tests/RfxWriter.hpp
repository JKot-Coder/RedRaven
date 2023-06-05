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
            RfxWriter(const std::vector<Common::ComPtr<Rfx::ICompileResult>>& compileResults)
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
            std::vector<Common::ComPtr<Rfx::ICompileResult>> compileResults_;
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

        class RfxWriter3 : public ApprovalTests::ApprovalWriter
        {
        public:
            ~RfxWriter3() = default;

            RfxWriter3(Common::RResult result, const U8String& output)
                : output_(output), result_(result)
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
            Common::RResult result_;
            U8String output_;
        };
    }
}