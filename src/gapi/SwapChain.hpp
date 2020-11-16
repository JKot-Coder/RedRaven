#pragma once

#include "gapi/Object.hpp"

#include "common/NativeWindowHandle.hpp"

namespace OpenDemo
{
    namespace Render
    {

        struct SwapChainDescription
        {
            uint32_t width;
            uint32_t height;

            NativeWindowHandle windowHandle;

        //    ResourceFormat resourceFormat;
            uint32_t bufferCount;
            bool isStereo;
        };

        class SwapChain final : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<SwapChain>;
            using SharedConstPtr = std::shared_ptr<const SwapChain>;

            SwapChain() = delete;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new SwapChain(name));
            }

            SwapChain(const U8String& name) : Object(Object::Type::SwapChain, name) { }

            friend class RenderContext;
        };
    }
}