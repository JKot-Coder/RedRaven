#pragma once

#include "ApprovalIntegration/BufferWriter.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace GAPI
{
    class Buffer;
}

namespace RR::Tests
{
    class BufferApprover
    {
    public:
        static void verify(const std::shared_ptr<GAPI::Buffer>& resource,
                           const ApprovalTests::Options& options = ApprovalTests::Options())
        {
            BufferWriter bufferWriter(resource);
            ApprovalTests::Approvals::verify(bufferWriter, options);
        }
    };
}