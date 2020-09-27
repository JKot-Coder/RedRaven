#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        
        class Texture final : public Resource
        {
        public:
            using SharedPtr = std::shared_ptr<Texture>;
            using SharedConstPtr = std::shared_ptr<const Texture>;
            using ConstSharedPtrRef = const SharedPtr&;

            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;
            Texture(const U8String& name)
                : Resource(Resource::Type::Texture, name)
            {
            }
        };

    }
}