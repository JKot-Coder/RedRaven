#include "Resource.hpp"

#include "gapi/Texture.hpp"
//#include "gapi/Buffer.hpp"

namespace OpenDemo
{
    namespace Render
    {
        template <>
        Texture& Resource::GetTyped<Texture>()
        {
            ASSERT(resourceType_ == Type::Texture)
            return dynamic_cast<Texture&>(*this);
        }
        /*
        template <>
        inline std::shared_ptr<Buffer> Resource::GetTyped<Buffer>()
        {
            ASSERT(resourceType_ == Type::Buffer)
            return std::dynamic_pointer_cast<Buffer>(shared_from_this());
        }*/
    }
}