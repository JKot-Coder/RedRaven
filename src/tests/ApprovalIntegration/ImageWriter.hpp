#pragma once

#include "gapi/Texture.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace RR
{
    namespace Tests
    {
        class ImageWriter : public ApprovalTests::ApprovalWriter
        {
        public:
            explicit ImageWriter(const GAPI::Texture::SharedPtr& resource,
                                 std::string fileExtensionWithDot = ".dds")
                : resource_(resource), fileExtensionWithDot_(fileExtensionWithDot)
            {
            }

            std::string getFileExtensionWithDot() const override
            {
                return fileExtensionWithDot_;
            }

            void write(std::string path) const override;

            void cleanUpReceived(std::string receivedPath) const override
            {
                remove(receivedPath.c_str());
            }

        private:
            GAPI::Texture::SharedPtr resource_;
            std::string fileExtensionWithDot_;
        };
    }
}