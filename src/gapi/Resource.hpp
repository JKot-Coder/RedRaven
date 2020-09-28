#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class Resource : public Object
        {
        public:
            Resource(const Resource&) = delete;
            Resource& operator=(const Resource&) = delete;
            Resource(Object::Type type, const U8String& name)
                : Object(type, name)
            {
            }
        };

    }
}