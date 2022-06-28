#include "BufferWriter.hpp"

#include "gapi/Buffer.hpp"

#include "common/OnScopeExit.hpp"

#include <filesystem>

namespace RR::Tests
{
    namespace
    {
        void saveToFile(const GAPI::Buffer::SharedPtr& resource, const std::filesystem::path& path)
        {
            ASSERT(resource);

            const auto& resourceDesc = resource->GetDescription();

            const auto dataPointer = static_cast<uint8_t*>(resource->Map());

            std::fstream file;

            ON_SCOPE_EXIT(
                {
                    resource->Unmap();
                    file.close();
                });

            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(path, std::ios::out | std::ios::binary);

            for (uint32_t subresourceIdx = 0; subresourceIdx < resourceDesc.GetNumSubresources(); subresourceIdx++)
            {
                const auto& subresourceFootprint = resource->GetSubresourceFootprintAt(subresourceIdx);

                ASSERT(subresourceFootprint.width * std::max(resourceDesc.GetStructSize(), 1u) == subresourceFootprint.rowSizeInBytes);

                auto subresourcePointer = reinterpret_cast<char*>(dataPointer) + subresourceFootprint.offset;

                file.write(subresourcePointer, subresourceFootprint.width);
            }
        }

    }

    void BufferWriter::write(std::string path) const
    {
        ASSERT(resource_);

        saveToFile(resource_, std::filesystem::path(path));
    }
}