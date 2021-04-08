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
        using Vector3u = Vector<3, uint32_t>;

        template <typename T>
        struct Box;

        using Box3u = Box<uint32_t>;
    }

    namespace GAPI
    {
        enum class CommandListType : uint32_t
        {
            Copy,
            Compute,
            Graphics
        };

        // https://docs.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles#command-list-api-restrictions
        class ICommandList
        {
        public:
            virtual void Close() = 0;

            virtual void CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer) = 0;
            virtual void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                          const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) = 0;

            virtual void CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture) = 0;
            virtual void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) = 0;
            virtual void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                      const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) = 0;

            virtual void UpdateTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData) = 0;
            virtual void ReadbackTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData) = 0;
        };

        class ICopyCommandList : public ICommandList
        {
        public:
        };

        class IComputeCommandList : public ICopyCommandList
        {
        public:
        };

        class IGraphicsCommandList : public IComputeCommandList
        {
        public:
            virtual ~IGraphicsCommandList() {};

            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandList : public Resource<IGraphicsCommandList>
        {
        public:
            using SharedPtr = std::shared_ptr<CommandList>;
            using SharedConstPtr = std::shared_ptr<const CommandList>;

            inline CommandListType GetCommandListType() const { return type_; };

            inline void Close() { return GetPrivateImpl()->Close(); }

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

            void CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer);
            void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                  const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes);
            void CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture);
            void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                        const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx);
            void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                              const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint);

            void UpdateTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData);
            void ReadbackTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData);

        private:
            template <class Deleter>
            static SharedPtr Create(const U8String& name, Deleter)
            {
                return SharedPtr(new CopyCommandList(CommandListType::Copy, name), Deleter());
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
            template <class Deleter>
            static SharedPtr Create(const U8String& name, Deleter)
            {
                return SharedPtr(new ComputeCommandList(CommandListType::Compute, name), Deleter());
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
            template <class Deleter>
            static SharedPtr Create(const U8String& name, Deleter)
            {
                return SharedPtr(new GraphicsCommandList(name), Deleter());
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

#if ENABLE_INLINE && !ENABLE_ASSERTS
#include "CommandList.inl"
#endif