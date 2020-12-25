#pragma once

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

    namespace GAPI
    {
        enum class CommandListType : uint32_t
        {
            Copy,
            Compute,
            Graphics
        };

        class CommandListInterface
        {
        public:
            virtual void Reset() = 0;
            virtual void Close() = 0;
        };

        class CopyCommandListInterface
        {
        public:
        };

        class ComputeCommandListInterface
        {
        public:
        };

        class GraphicsCommandListInterface
        {
        public:
            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandList : public Object, public CommandListInterface
        {
        public:
            inline void Reset() override { getImplementation().Reset(); }
            inline void Close() override { getImplementation().Close(); }

            inline CommandListType GetCommandListType() const { return type_; };

        private:
            inline CommandListInterface& getImplementation()
            {
                ASSERT(privateImpl_);

                return *(static_cast<CommandListInterface*>(privateImpl_));
            }

        protected:
            CommandList(CommandListType type, const U8String& name)
                : Object(Object::Type::CommandList, name),
                  type_(type)
            {
            }

            CommandListType type_;
        };

        class CopyCommandList final : public CommandList, public CopyCommandListInterface
        {
        public:
            using SharedPtr = std::shared_ptr<CopyCommandList>;
            using SharedConstPtr = std::shared_ptr<const CopyCommandList>;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new CopyCommandList(name));
            }

            CopyCommandList(const U8String& name)
                : CommandList(CommandListType::Copy, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };

        class ComputeCommandList final : public CommandList, public CopyCommandListInterface, ComputeCommandListInterface
        {
        public:
            using SharedPtr = std::shared_ptr<ComputeCommandList>;
            using SharedConstPtr = std::shared_ptr<const ComputeCommandList>;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new ComputeCommandList(name));
            }

            ComputeCommandList(const U8String& name)
                : CommandList(CommandListType::Compute, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };

        class GraphicsCommandList final : public CommandList,
                                          public CopyCommandListInterface,
                                          public ComputeCommandListInterface,
                                          public GraphicsCommandListInterface
        {
        public:
            using SharedPtr = std::shared_ptr<GraphicsCommandList>;
            using SharedConstPtr = std::shared_ptr<const GraphicsCommandList>;

            inline void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override { getImplementation().ClearRenderTargetView(renderTargetView, color); }

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new GraphicsCommandList(name));
            }

            GraphicsCommandList(const U8String& name)
                : CommandList(CommandListType::Graphics, name)
            {
            }

            inline GraphicsCommandListInterface& getImplementation()
            {
                ASSERT(privateImpl_);

                return *(static_cast<GraphicsCommandListInterface*>(privateImpl_));
            }

        private:
            friend class Render::RenderContext;
        };
    }
}