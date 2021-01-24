#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Result.hpp"

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
        struct TextureSubresourceFootprint
        {
            TextureSubresourceFootprint(void* data, size_t size, size_t rowPitch, size_t depthPitch)
                : data(data), size(size), rowPitch(rowPitch), depthPitch(depthPitch) { }

            void* data;
            size_t size;
            size_t rowPitch;
            size_t depthPitch;
        };

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
            virtual Result Close() = 0;

            virtual void CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer) = 0;
            virtual void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                          const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) = 0;

            virtual void CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture) = 0;
            virtual void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) = 0;
            virtual void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                      const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) = 0;

            virtual void UpdateTextureData(const std::shared_ptr<Texture>& texture, const std::vector<TextureSubresourceFootprint>& subresourceFootprint) = 0;
            virtual void UpdateSubresourceData(const std::shared_ptr<Texture>& texture, uint32_t firstSubresource, const std::vector<TextureSubresourceFootprint>& subresourceFootprint) = 0;
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

            inline Result Close() { return GetPrivateImpl()->Close(); }

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

            inline void CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer)
            {
                GetPrivateImpl()->CopyBuffer(sourceBuffer, destBuffer);
            }

            inline void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                         const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes)
            {
                GetPrivateImpl()->CopyBufferRegion(sourceBuffer, sourceOffset, destBuffer, destOffset, numBytes);
            }

            inline void CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture)
            {
                GetPrivateImpl()->CopyTexture(sourceTexture, destTexture);
            }

            inline void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                               const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx)
            {
                GetPrivateImpl()->CopyTextureSubresource(sourceTexture, sourceSubresourceIdx, destTexture, destSubresourceIdx);
            }

            inline void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                     const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint)
            {
                GetPrivateImpl()->CopyTextureSubresourceRegion(sourceTexture, sourceSubresourceIdx, sourceBox, destTexture, destSubresourceIdx, destPoint);
            }

            inline void UpdateTextureData(const std::shared_ptr<Texture>& texture, const std::vector<TextureSubresourceFootprint>& subresourceFootprint)
            {
                GetPrivateImpl()->UpdateTextureData(texture, subresourceFootprint);
            }

            inline void UpdateSubresourceData(const std::shared_ptr<Texture>& texture, uint32_t firstSubresource, const std::vector<TextureSubresourceFootprint>& subresourceFootprint)
            {
                GetPrivateImpl()->UpdateSubresourceData(texture, firstSubresource, subresourceFootprint);
            }

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