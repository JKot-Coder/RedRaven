#include "SourceManager.hpp"

#include "common/ErrorNo.hpp"
#include "common/LinearAllocator.hpp"
#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"
#include "parse_tools/core/CompileContext.hpp"
#include "parse_tools/core/SourceLocation.hpp"

#include <filesystem>
#include <fstream>

namespace RR::ParseTools
{
    namespace
    {
        namespace fs = std::filesystem;

#define ASSERT_ON_FALSE(x)    \
    if (!(x))                 \
    {                         \
        ASSERT(false);        \
        return RResult::Fail; \
    }
    }

    SourceManager::SourceManager(const std::shared_ptr<CompileContext>& context)
        : context_(context) { ASSERT(context); }

    SourceManager::~SourceManager() { }

    RResult SourceManager::readFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& outSourceFile)
    {
        ASSERT_ON_FALSE(pathInfo.hasUniqueIdentity());
        const auto& path = fs::path(pathInfo.uniqueIdentity);

        if (!fs::exists(path))
            return RResult::NotFound;

        std::ifstream stream(pathInfo.uniqueIdentity, std::ios::binary);

        if (!stream)
        {
            context_->sink.Diagnose(Diagnostics::cannotOpenFile, pathInfo.uniqueIdentity, Common::GetErrorMessage());
            return RResult::CannotOpen;
        }

        ON_SCOPE_EXIT({ stream.close(); });

        stream.seekg(0, stream.end);
        uint64_t sizeInBytes = ((uint64_t)stream.tellg());
        stream.seekg(0, stream.beg);

        const uint64_t MaxFileSize = 0x40000000; // 1 Gib
        if (sizeInBytes > MaxFileSize)
            return RResult::Fail; // It's too large to fit in memory.

        const auto content = (char*)context_->allocator.Allocate(sizeInBytes + 1);
        stream.read(content, sizeInBytes);

        content[sizeInBytes] = '\0';

        if (!stream) // If not all read just return an error
            return RResult::Fail;

        outSourceFile = std::make_shared<SourceFile>(pathInfo);
        outSourceFile->SetContent(UnownedStringSlice(content, content + sizeInBytes));

        return RResult::Ok;
    }

    RResult SourceManager::findSourceFileByUniqueIdentity(const std::string& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const
    {
        const auto findIt = sourceFileMap_.find(uniqueIdentity);
        if (findIt != sourceFileMap_.end())
        {
            sourceFile = findIt->second;
            return RResult::Ok;
        }

        return RResult::NotFound;
    }

    void SourceManager::addSourceFile(const std::string& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile)
    {
        ASSERT(RR_FAILED(findSourceFileByUniqueIdentity(uniqueIdentity, sourceFile)));
        sourceFileMap_.emplace(uniqueIdentity, sourceFile);
    }

    std::shared_ptr<SourceFile> SourceManager::CreateFileFromString(const PathInfo& pathInfo, const std::string& content)
    {
        ASSERT(pathInfo.type == PathInfo::Type::CommandLine ||
               pathInfo.type == PathInfo::Type::FromString ||
               pathInfo.type == PathInfo::Type::TokenPaste);

        const auto contentSize = content.length();
        const auto contentPtr = (char*)context_->allocator.Allocate(contentSize + 1);
        std::memcpy(contentPtr, content.c_str(), contentSize);
        contentPtr[contentSize] = '\0';

        const auto& sourceFile = std::make_shared<SourceFile>(pathInfo);
        sourceFile->SetContent(UnownedStringSlice(contentPtr, contentPtr + contentSize));
        sourceFiles_.push_back(sourceFile);

        return sourceFile;
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