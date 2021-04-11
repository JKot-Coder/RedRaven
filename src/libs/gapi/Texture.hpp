#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {

        /*
        struct TextureSubresourceData
        {
            TextureSubresourceData(void* data, size_t size, size_t rowPitch, size_t depthPitch)
                : data(data), size(size), rowPitch(rowPitch), depthPitch(depthPitch) { }

            void* data;
            size_t size;
            size_t rowPitch;
            size_t depthPitch;
        };*/

        class CpuResourceData : public std::enable_shared_from_this<CpuResourceData>
        {
        public:
            using SharedPtr = std::shared_ptr<CpuResourceData>;
            using SharedConstPtr = std::shared_ptr<const CpuResourceData>;

            struct SubresourceFootprint
            {
                SubresourceFootprint() = default;
                SubresourceFootprint(size_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t numRows, uint32_t rowSizeInBytes, size_t rowPitch, size_t depthPitch)
                    : offset(offset), width(width), height(height), depth(depth), numRows(numRows), rowSizeInBytes(rowSizeInBytes), rowPitch(rowPitch), depthPitch(depthPitch) { }

                bool isComplatable(const SubresourceFootprint& other) const
                {
                    return (numRows == other.numRows) &&
                           (rowSizeInBytes == other.rowSizeInBytes);
                }

                size_t offset;
                uint32_t width;
                uint32_t height;
                uint32_t depth;
                uint32_t numRows;
                size_t rowSizeInBytes;
                size_t rowPitch;
                size_t depthPitch;
            };

            CpuResourceData(const std::shared_ptr<MemoryAllocation>& allocation, const GpuResourceDescription& resourceDescription, const std::vector<SubresourceFootprint>& subresourceFootprints, uint32_t firstSubresource)
                : allocation_(allocation),
                  resourceDescription_(resourceDescription),
                  subresourceFootprints_(subresourceFootprints),
                  firstSubresource_(firstSubresource)
            {
                ASSERT(resourceDescription.GetDimension() != GpuResourceDimension::Unknown);
                ASSERT(resourceDescription.GetFormat() != GpuResourceFormat::Unknown);

                ASSERT(allocation);
                ASSERT(GetNumSubresources() > 0);
            };

            inline std::shared_ptr<MemoryAllocation> GetAllocation() const { return allocation_; }
            inline uint32_t GetFirstSubresource() const { return firstSubresource_; }
            inline size_t GetNumSubresources() const { return subresourceFootprints_.size(); }
            inline const GpuResourceDescription& GetResourceDescription() const { return resourceDescription_; }
            inline const SubresourceFootprint& GetSubresourceFootprintAt(uint32_t index) const { return subresourceFootprints_[index]; }
            inline const std::vector<SubresourceFootprint>& GetSubresourceFootprints() const { return subresourceFootprints_; }

            void CopyDataFrom(const GAPI::CpuResourceData::SharedPtr& source);

        private:
            std::shared_ptr<MemoryAllocation> allocation_;
            std::vector<SubresourceFootprint> subresourceFootprints_;
            GpuResourceDescription resourceDescription_;
            uint32_t firstSubresource_;
        };

        class Texture final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Texture>;
            using SharedConstPtr = std::shared_ptr<const Texture>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            std::shared_ptr<ShaderResourceView> GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible);
            std::shared_ptr<RenderTargetView> GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible);
            std::shared_ptr<DepthStencilView> GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible);
            std::shared_ptr<UnorderedAccessView> GetUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible);

        private:
            template <class Deleter>
            static SharedPtr Create(
                const GpuResourceDescription& description,
                GpuResourceCpuAccess cpuAccess,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new Texture(description, cpuAccess, name), Deleter());
            }

            Texture(const GpuResourceDescription& description, GpuResourceCpuAccess cpuAccess, const U8String& name);

        private:
            friend class Render::RenderContext;
        };
    }
}