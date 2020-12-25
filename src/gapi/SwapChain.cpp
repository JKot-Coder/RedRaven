#include "SwapChain.hpp"

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {

        SwapChain::SwapChain(const SwapChainDescription& description, const U8String& name)
            : Object(Object::Type::SwapChain, name),
              description_(description)
        {
            ASSERT(description.width > 0)
            ASSERT(description.height > 0)
            ASSERT(description.isStereo == false)
            ASSERT(description.resourceFormat != ResourceFormat::Unknown)
            //TODO fix it
            ASSERT(description.windowHandle != 0)
        }

    }
}