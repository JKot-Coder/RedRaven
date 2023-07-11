#include "SourceManager.hpp"

#include "common/Result.hpp"
#include "core/SourceLocation.hpp"
#include "common/OnScopeExit.hpp"

#include <fstream>
#include <filesystem>

namespace RR::Rfx
{
    namespace
    {
        namespace fs = std::filesystem;

#define ASSERT_ON_FALSE(x) \
    if (!x)                     \
    {                           \
        ASSERT(false);          \
        return RResult::Fail;   \
    }

        RResult readFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& outSourceFile)
        {
            ASSERT_ON_FALSE(pathInfo.hasUniqueIdentity());
            const auto& path = fs::path(pathInfo.uniqueIdentity);

            if (!fs::exists(path))
                return RResult::NotFound;

            std::ifstream stream(pathInfo.uniqueIdentity, std::ios::binary);

            if (!stream)
                return RResult::Fail;

            ON_SCOPE_EXIT({ stream.close(); });

            stream.seekg(0, stream.end);
            uint64_t sizeInBytes = stream.tellg();
            stream.seekg(0, stream.beg);

            const uint64_t MaxFileSize = 0x40000000; // 1 Gib
            if (sizeInBytes > MaxFileSize)
                return RResult::Fail; // It's too large to fit in memory.

            U8String content;
            content.resize(sizeInBytes);
            stream.read(&content[0], content.size());

            if (!stream) // If not all read just return an error
                return RResult::Fail;

            outSourceFile = std::make_shared<SourceFile>(pathInfo);
            outSourceFile->SetContent(std::move(content));

            return RfxResult::Ok;
        }
    }

    RResult SourceManager::findSourceFileByUniqueIdentity(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const
    {
        const auto findIt = sourceFileMap_.find(uniqueIdentity);
        if (findIt != sourceFileMap_.end())
        {
            sourceFile = findIt->second;
            return RResult::Ok;
        }

        return RResult::NotFound;
    }

    void SourceManager::addSourceFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile)
    {
        ASSERT(RR_FAILED(findSourceFileByUniqueIdentity(uniqueIdentity, sourceFile)));
        sourceFileMap_.emplace(uniqueIdentity, sourceFile);
    }

    RResult SourceManager::LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile)
    {
        ASSERT_ON_FALSE(pathInfo.hasUniqueIdentity());

        RR_RETURN_ON_SUCESS(findSourceFileByUniqueIdentity(pathInfo.uniqueIdentity, sourceFile));
        RR_RETURN_ON_FAIL(readFile(pathInfo, sourceFile));

        addSourceFile(pathInfo.uniqueIdentity, sourceFile);
        return RResult::Ok;
    }
}