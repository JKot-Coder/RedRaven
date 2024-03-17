#pragma once

#include <filesystem>
#include <uuid.h>

namespace RR::AssetImporter
{
    class Meta
    {
        public:
            using UUID = uuids::uuid;

        public:
            static std::shared_ptr<Meta> CreateForPath(const std::filesystem::path& path);

        private:
            UUID uuid;
    };
}