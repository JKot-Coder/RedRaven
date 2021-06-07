#pragma once

#include "gapi/Texture.hpp"

#include "ApprovalIntegration/ImageWriter.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace RR
{
    namespace Tests
    {
        class ImageApprover
        {
        public:
            static void verify(const GAPI::CpuResourceData::SharedPtr& resource,
                               const ApprovalTests::Options& options = ApprovalTests::Options())
            {
                ImageWriter image_writer(resource);
                ApprovalTests::Approvals::verify(image_writer, options);
            }
        };
    }
}