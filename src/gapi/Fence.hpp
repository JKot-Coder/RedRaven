#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class Fence final : public Object, public std::enable_shared_from_this<Fence>
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;
        };

    }
}