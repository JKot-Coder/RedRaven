#pragma once

namespace OpenDemo
{
    namespace Render
    {
        class RenderTargetView final : public std::enable_shared_from_this<RenderTargetView>
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<const RenderTargetView>;
            using ConstSharedPtrRef = const SharedPtr&;
        };
    }
}