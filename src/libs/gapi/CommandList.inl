namespace OpenDemo
{
    namespace GAPI
    {
        INLINE void CopyCommandList::CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                                      const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes)
        {
            ASSERT(sourceBuffer);
            ASSERT(destBuffer);

            GetPrivateImpl()->CopyBufferRegion(sourceBuffer, sourceOffset, destBuffer, destOffset, numBytes);
        }

        INLINE void CopyCommandList::CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer)
        {
            ASSERT(sourceBuffer);
            ASSERT(destBuffer);

            GetPrivateImpl()->CopyBuffer(sourceBuffer, destBuffer);
        }

        INLINE void CopyCommandList::CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture)
        {
            ASSERT(sourceTexture);
            ASSERT(destTexture);

            GetPrivateImpl()->CopyTexture(sourceTexture, destTexture);
        }

        INLINE void CopyCommandList::CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                            const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx)
        {
#ifdef ENABLE_ASSERTS
            ASSERT(sourceTexture);
            ASSERT(destTexture);

            const auto& sourceDesc = sourceTexture->GetDescription();
            const auto& destDesc = destTexture->GetDescription();
            ASSERT(sourceSubresourceIdx < sourceDesc.GetNumSubresources());
            ASSERT(destSubresourceIdx < destDesc.GetNumSubresources());
#endif

            GetPrivateImpl()->CopyTextureSubresource(sourceTexture, sourceSubresourceIdx, destTexture, destSubresourceIdx);
        }

        namespace
        {
#ifdef ENABLE_ASSERTS
            bool checkTextureRegion(const GpuResourceDescription& desc, uint32_t subresourceIdx, const Box3u& region)
            {
                auto result = true;

                const auto mipLevel = desc.GetSubresourceMipLevel(subresourceIdx);

                const auto mipWidth = desc.GetWidth(mipLevel);
                const auto mipHeight = desc.GetHeight(mipLevel);
                const auto mipDepth = desc.GetDepth(mipLevel);

                const auto regionMinPos = region.GetPosition();
                const auto regionMaxPos = region.GetPosition() + region.GetSize();

                result &= regionMinPos.x < mipWidth;
                result &= regionMinPos.y < mipHeight;
                result &= regionMinPos.z < mipDepth;

                result &= regionMaxPos.x <= mipWidth;
                result &= regionMaxPos.y <= mipHeight;
                result &= regionMaxPos.z <= mipDepth;

                return result;
            }
#endif
        }

        INLINE void CopyCommandList::CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                                  const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint)
        {
            ASSERT(sourceTexture);
            ASSERT(destTexture);

#ifdef ENABLE_ASSERTS
            const auto& sourceDesc = sourceTexture->GetDescription();
            const auto& destDesc = destTexture->GetDescription();
            ASSERT(sourceSubresourceIdx < sourceDesc.GetNumSubresources());
            ASSERT(destSubresourceIdx < destDesc.GetNumSubresources());

            ASSERT(checkTextureRegion(sourceDesc, sourceSubresourceIdx, sourceBox));
            ASSERT(checkTextureRegion(destDesc, destSubresourceIdx, Box3u(destPoint, sourceBox.GetSize())));
#endif

            GetPrivateImpl()->CopyTextureSubresourceRegion(sourceTexture, sourceSubresourceIdx, sourceBox, destTexture, destSubresourceIdx, destPoint);
        }

        INLINE void CopyCommandList::UpdateTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData)
        {
            ASSERT(texture);
            ASSERT(textureData);

            GetPrivateImpl()->UpdateTexture(texture, textureData);
        }

        INLINE void CopyCommandList::ReadbackTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<CpuResourceData>& textureData)
        {
            ASSERT(texture);
            ASSERT(textureData);

            GetPrivateImpl()->ReadbackTexture(texture, textureData);
        }

    }
}