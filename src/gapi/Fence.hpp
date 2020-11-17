#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class FenceInterface
        {

        };

        class Fence final : public Object, FenceInterface
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new Fence(name));
            }

            Fence(const U8String& name)
                : Object(Object::Type::Fence, name)
            {
            }

        private:
            friend class RenderContext;
        };

    }
}