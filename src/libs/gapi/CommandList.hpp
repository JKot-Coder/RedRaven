#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

namespace RR
{
    namespace Common
    {
        template <size_t Len, typename T>
        struct Vector;

        using Vector4 = Vector<4, float>;
        using Vector3u = Vector<3, uint32_t>;
        using Vector4u = Vector<4, uint32_t>;

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
            virtual ~ICommandList() = default;
            virtual void Close() = 0;

            // ---------------------------------------------------------------------------------------------
            // Copy command list
            // ---------------------------------------------------------------------------------------------

            virtual void CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest) = 0;
            virtual void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                          const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) = 0;
            virtual void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) = 0;
            virtual void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                      const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) = 0;

            virtual void UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData) = 0;
            virtual void ReadbackGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData) = 0;

            // ---------------------------------------------------------------------------------------------
            // Compute command list
            // ---------------------------------------------------------------------------------------------

            virtual void ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) = 0;
            virtual void ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) = 0;

            // ---------------------------------------------------------------------------------------------
            // Graphics command list
            // ---------------------------------------------------------------------------------------------

            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandList : public Resource<ICommandList>
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

            void CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest);

            void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                  const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes);
            void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                        const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx);
            void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                              const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint);

            void UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData);
            void ReadbackGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData);

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
            friend class Render::DeviceContext;
        };

        class ComputeCommandList : public CopyCommandList
        {
        public:
            using SharedPtr = std::shared_ptr<ComputeCommandList>;
            using SharedConstPtr = std::shared_ptr<const ComputeCommandList>;

            inline void ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) { GetPrivateImpl()->ClearUnorderedAccessViewUint(unorderedAcessView, clearValue); }
            inline void ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) { GetPrivateImpl()->ClearUnorderedAccessViewFloat(unorderedAcessView, clearValue); };

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
            friend class Render::DeviceContext;
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
            friend class Render::DeviceContext;
        };
    }
}

#if ENABLE_INLINE
#include "CommandList.inl"
#endif