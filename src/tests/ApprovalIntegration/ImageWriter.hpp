#pragma once

#include "gapi/Texture.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace OpenDemo
{
    namespace Tests
    {
        class ImageWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            explicit ImageWriter(const GAPI::CpuResourceData::SharedPtr& resource,
                                 std::string fileExtensionWithDot = ".ktx")
                : resource_(resource), fileExtensionWithDot_(fileExtensionWithDot)
            {
            }

            std::string getFileExtensionWithDot() const override
            {
                return fileExtensionWithDot_;
            }

            void write(std::string path) const override
            {
                ASSERT_MSG(false, "Implement");
            }

            void cleanUpReceived(std::string receivedPath) const override
            {
                remove(receivedPath.c_str());
            }

        private:
            GAPI::CpuResourceData::SharedPtr resource_;
            std::string fileExtensionWithDot_;
        };
    }
}