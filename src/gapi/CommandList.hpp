#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

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
            // TODO result;
            virtual void Reset() = 0;
            // TODO result;
            virtual void Close() = 0;
        };

        class CopyCommandListInterface : public CommandListInterface
        {
        public:
        };

        class ComputeCommandListInterface : public CopyCommandListInterface
        {
        public:
        };

        class GraphicsCommandListInterface : public ComputeCommandListInterface
        {
        public:
            virtual ~GraphicsCommandListInterface() {};

            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandList : public Resource<GraphicsCommandListInterface>
        {
        public:
            using SharedPtr = std::shared_ptr<CommandList>;
            using SharedConstPtr = std::shared_ptr<const CommandList>;

            inline CommandListType GetCommandListType() const { return type_; };

            inline void Reset() { GetPrivateImpl()->Reset(); }
            inline void Close() { GetPrivateImpl()->Close(); }

        protected:
            CommandList(CommandListType type, const U8String& name)
                : Resource(Object::Type::CommandList, name),
                  type_(type)
            {
            }

            CommandListType type_;
        };

        class CopyCommandList : public CommandList
        {
        public:
            using SharedPtr = std::shared_ptr<CopyCommandList>;
            using SharedConstPtr = std::shared_ptr<const CopyCommandList>;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new CopyCommandList(CommandListType::Copy, name));
            }

        protected:
            CopyCommandList(CommandListType type, const U8String& name)
                : CommandList(type, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };

        class ComputeCommandList : public CopyCommandList
        {
        public:
            using SharedPtr = std::shared_ptr<ComputeCommandList>;
            using SharedConstPtr = std::shared_ptr<const ComputeCommandList>;

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new ComputeCommandList(CommandListType::Compute, name));
            }

        protected:
            ComputeCommandList(CommandListType type, const U8String& name)
                : CopyCommandList(type, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };

        class GraphicsCommandList final : public ComputeCommandList
        {
        public:
            using SharedPtr = std::shared_ptr<GraphicsCommandList>;
            using SharedConstPtr = std::shared_ptr<const GraphicsCommandList>;

            inline void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) { GetPrivateImpl()->ClearRenderTargetView(renderTargetView, color); }

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new GraphicsCommandList(name));
            }

        protected:
            GraphicsCommandList(const U8String& name)
                : ComputeCommandList(CommandListType::Graphics, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };
    }
}