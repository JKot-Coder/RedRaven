#pragma once 

#include "common/Singleton.hpp"
#include <unordered_map>
#include <filesystem>

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::AssetImporter
{
    class Processor;
    
    class AssetImporter : public Common::Singleton<AssetImporter>
    {
        public:
            void Register(Processor& Processorimporter);
            void BuildBundle(const std::filesystem::path& path);
            
            Common::RResult ProcessAsset(const std::filesystem::path& path);

        private:
            void importAsset(const std::filesystem::path& path);

        private:
            std::unordered_map<U8String, Processor*> processors_;
    };
}
