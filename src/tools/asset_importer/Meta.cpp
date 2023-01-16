#include "Meta.hpp"

namespace RR::AssetImporter
{
    std::shared_ptr<Meta> Meta::CreateForPath(const std::filesystem::path& path)
    {
        std::ignore = path;
        return nullptr;
    }
}