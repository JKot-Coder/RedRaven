#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class Fence final : public Resource, public std::enable_shared_from_this<Fence>
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;
            using ConstSharedPtrRef = const SharedPtr&;
        };

    }
}