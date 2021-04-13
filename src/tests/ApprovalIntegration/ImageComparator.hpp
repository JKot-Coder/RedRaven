#pragma once

#include "gapi/Texture.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

namespace OpenDemo
{
    namespace Tests
    {
        class ImageComparator : public ApprovalTests::ApprovalComparator
        {
        public:
            bool contentsAreEquivalent(std::string receivedPath,
                                       std::string approvedPath) const override;
        };
    }
}