#include "SourceManager.hpp"

#include "common/Result.hpp"
#include "core/SourceLocation.hpp"

namespace RR::Rfx
{
    std::shared_ptr<SourceFile> SourceManager::findSourceFileByUniqueIdentity(const U8String& uniqueIdentity) const
    {
        const auto findIt = sourceFileMap_.find(uniqueIdentity);
        if (findIt != sourceFileMap_.end())
            return findIt->second;

        return nullptr;
    }

    std::shared_ptr<SourceFile> SourceManager::createSourceFileWithString()
    {
    }

    RResult SourceManager::LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile)
    {
        if (pathInfo.hasUniqueIdentity())
        {
            if (sourceFile = findSourceFileByUniqueIdentity(pathInfo.getMostUniqueIdentity()))
                return RResult::Ok;
        }

        return includeSystem_->LoadFile(pathInfo, sourceFile);
    }
}