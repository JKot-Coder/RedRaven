#pragma once

#include "gapi/Texture.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace OpenDemo
{
    namespace Tests
    {
        class ImageApprovalComparator : public ApprovalTests::ApprovalComparator
        {
        public:
            bool contentsAreEquivalent(std::string receivedPath,
                                       std::string approvedPath) const override
            {
                ASSERT_MSG(false, "implemet");
                return true;
            }

            static void verify(const GAPI::CpuResourceData& resource,
                               const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                ImageApprovalWriter image_writer(resource);
                ApprovalTests::Approvals::verify(image_writer, options);
            }
        };
    }
}