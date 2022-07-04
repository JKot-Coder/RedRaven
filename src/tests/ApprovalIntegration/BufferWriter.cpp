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

            const auto& resoucerData = resource->GetResourceData();

            std::fstream file;
            ON_SCOPE_EXIT({ file.close(); });

            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(path, std::ios::out | std::ios::binary);

            auto dataPointer = resoucerData->Data();
            for (uint32_t subresourceIdx = 0; subresourceIdx < resourceDesc.GetNumSubresources(); subresourceIdx++)
            {
                const auto& subresourceFootprint = resoucerData->GetSubresourceFootprintAt(subresourceIdx);

                ASSERT(subresourceFootprint.width * std::max(resourceDesc.GetStructSize(), 1u) == subresourceFootprint.rowSizeInBytes);

                auto subresourcePointer = static_cast<char*>(dataPointer) + subresourceFootprint.offset;

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