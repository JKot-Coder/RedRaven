#pragma once

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace RR::GAPI
{
    class Buffer;
}

namespace RR::Tests
{
    class BufferWriter : public ApprovalTests::ApprovalWriter
    {
    public:
        explicit BufferWriter(const std::shared_ptr<RR::GAPI::Buffer>& resource,
                              std::string fileExtensionWithDot = ".bin")
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
        std::shared_ptr<RR::GAPI::Buffer> resource_;
        std::string fileExtensionWithDot_;
    };
}