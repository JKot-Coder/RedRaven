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
            const auto& resourceFootprints = resource->GetSubresourceFootprints();
            const auto resoucerData = GAPI::GpuResourceDataGuard(resource);

            std::fstream file;
            ON_SCOPE_EXIT({ file.close(); });

            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(path, std::ios::out | std::ios::binary);

            for (uint32_t subresourceIdx = 0; subresourceIdx < resourceFootprints.size(); subresourceIdx++)
            {
                const auto& subresourceFootprint = resourceFootprints[subresourceIdx];

                auto subresourcePointer = static_cast<char*>(resoucerData.Data()) + subresourceFootprint.offset;
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