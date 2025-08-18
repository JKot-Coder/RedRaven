#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

#include "math/ForwardDeclarations.hpp"
#include "gapi/commands/Command.hpp"

// TODO temporary
#include <any>

namespace RR
{
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
            virtual void Reset() = 0;
            virtual void Close() = 0;

            // TODO temporary
            virtual std::any GetNativeHandle() const = 0;

            // ---------------------------------------------------------------------------------------------
            // Copy command list
            // ---------------------------------------------------------------------------------------------

            virtual void CopyGpuResource(const eastl::shared_ptr<GpuResource>& source, const eastl::shared_ptr<GpuResource>& dest) = 0;
            virtual void CopyBufferRegion(const eastl::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                          const eastl::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) = 0;
            virtual void CopyTextureSubresource(const eastl::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                const eastl::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) = 0;
            virtual void CopyTextureSubresourceRegion(const eastl::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                      const eastl::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) = 0;

            // ---------------------------------------------------------------------------------------------
            // Compute command list
            // ---------------------------------------------------------------------------------------------

            virtual void ClearUnorderedAccessViewUint(const eastl::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) = 0;
            virtual void ClearUnorderedAccessViewFloat(const eastl::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) = 0;

            // ---------------------------------------------------------------------------------------------
            // Graphics command list
            // ---------------------------------------------------------------------------------------------

            virtual void SetFrameBuffer(const eastl::shared_ptr<Framebuffer>& framebuffer) = 0;
            virtual void SetIndexBuffer(const eastl::shared_ptr<Buffer>& buffer, size_t offset = 0) = 0;
            virtual void ClearRenderTargetView(const eastl::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };

        class CommandList : public Resource<ICommandList>
        {
        public:
            using SharedPtr = eastl::shared_ptr<CommandList>;
            using SharedConstPtr = eastl::shared_ptr<const CommandList>;

            inline CommandListType GetCommandListType() const { return type_; };
            inline std::any GetNativeHandle() const { return GetPrivateImpl()->GetNativeHandle(); }

            inline void Reset() { return GetPrivateImpl()->Reset(); }
            inline void Close() { return GetPrivateImpl()->Close(); }

        protected:
            CommandList(CommandListType type, const std::string& name)
                : Resource(Type::CommandList, name),
                  type_(type)
            {
            }

            CommandListType type_;
        };

        class CopyCommandList : public CommandList
        {
        public:
            using SharedPtr = eastl::shared_ptr<CopyCommandList>;
            using SharedConstPtr = eastl::shared_ptr<const CopyCommandList>;

            void CopyGpuResource(const eastl::shared_ptr<GpuResource>& source, const eastl::shared_ptr<GpuResource>& dest);

            void CopyBufferRegion(const eastl::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                  const eastl::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes);
            void CopyTextureSubresource(const eastl::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                        const eastl::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx);
            void CopyTextureSubresourceRegion(const eastl::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                              const eastl::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint);

        private:
            static SharedPtr Create(const std::string& name)
            {
                return SharedPtr(new CopyCommandList(CommandListType::Copy, name));
            }

        protected:
            CopyCommandList(CommandListType type, const std::string& name)
                : CommandList(type, name)
            {
            }

        private:
            friend class Render::DeviceContext;
        };

        class ComputeCommandList : public CopyCommandList
        {
        public:
            using SharedPtr = eastl::shared_ptr<ComputeCommandList>;
            using SharedConstPtr = eastl::shared_ptr<const ComputeCommandList>;

            inline void ClearUnorderedAccessViewUint(const eastl::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) { GetPrivateImpl()->ClearUnorderedAccessViewUint(unorderedAcessView, clearValue); }
            inline void ClearUnorderedAccessViewFloat(const eastl::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) { GetPrivateImpl()->ClearUnorderedAccessViewFloat(unorderedAcessView, clearValue); };

        private:
            static SharedPtr Create(const std::string& name)
            {
                return SharedPtr(new ComputeCommandList(CommandListType::Compute, name));
            }

        protected:
            ComputeCommandList(CommandListType type, const std::string& name)
                : CopyCommandList(type, name)
            {
            }

        private:
            friend class Render::DeviceContext;
        };

        class GraphicsCommandList final : public ComputeCommandList
        {
        public:
            using SharedPtr = eastl::shared_ptr<GraphicsCommandList>;
            using SharedConstPtr = eastl::shared_ptr<const GraphicsCommandList>;

            inline void SetFrameBuffer(const eastl::shared_ptr<Framebuffer>& framebuffer) { GetPrivateImpl()->SetFrameBuffer(framebuffer); }
            inline void SetIndexBuffer(const eastl::shared_ptr<Buffer>& buffer, size_t offset = 0) { GetPrivateImpl()->SetIndexBuffer(buffer, offset); }
            inline void ClearRenderTargetView(const eastl::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) { GetPrivateImpl()->ClearRenderTargetView(renderTargetView, color); }

        private:
            static SharedPtr Create(const std::string& name)
            {
                return SharedPtr(new GraphicsCommandList(name));
            }

        protected:
            GraphicsCommandList(const std::string& name)
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