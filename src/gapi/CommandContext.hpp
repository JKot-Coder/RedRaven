#pragma once

#include "gapi/CommandContextInterface.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Common
    {
        template <size_t Len, typename T>
        struct Vector;

        using Vector4 = Vector<4, float>;
    }

    namespace Render
    {

        class CommandContextInterface
        {
        public:
            virtual void Reset() = 0;
            virtual void Close() = 0;

            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandContext final : public Object, public CommandContextInterface
        {
        public:
            using SharedPtr = std::shared_ptr<CommandContext>;
            using SharedConstPtr = std::shared_ptr<const CommandContext>;

            CommandContext() = delete;

            inline void Reset() override { getImplementation().Reset(); }
            inline void Close() override { getImplementation().Close(); }

            inline void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override { getImplementation().ClearRenderTargetView(renderTargetView, color); }

            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new CommandContext(name));
            }

        private:
            CommandContext(const U8String& name)
                : Object(Object::Type::CommandContext, name)
            {
            }

            inline CommandContextInterface& getImplementation()
            {
                ASSERT(privateImpl_);

                return *(static_cast<CommandContextInterface*>(privateImpl_));
            }
        };
    }
}