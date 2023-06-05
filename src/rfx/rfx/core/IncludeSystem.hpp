#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Common
    {
        class LinearAllocator;
        enum class RResult : int32_t;
    }

    namespace Rfx
    {
        struct PathInfo;
        class IRfxFileSystem;
        class SourceFile;

        /* A helper class that builds basic include handling on top of searchDirectories/fileSystemExt */ //TODO
        class IncludeSystem final
        {
        public:
            IncludeSystem(const std::shared_ptr<IRfxFileSystem>& fileSystemExt)
                : fileSystemExt_(fileSystemExt)
            {
                ASSERT(fileSystemExt);
            };

            /* IncludeSystem(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt)
                    : m_searchDirectories(searchDirectories),
                      m_fileSystemExt(fileSystemExt)
                {
                }*/

            Common::RResult FindFile(const U8String& pathToInclude, const U8String& pathIncludedFrom, PathInfo& outPathInfo) const;
            Common::RResult LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& outSourceFile);

            std::shared_ptr<SourceFile> CreateFileFromString(const PathInfo& pathInfo, const U8String& content) const;
            /*
            SlangResult findAndLoadFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo, ComPtr<ISlangBlob>& outBlob);

            SearchDirectoryList* getSearchDirectoryList() const { return m_searchDirectories; }
            ISlangFileSystemExt* getFileSystem() const { return m_fileSystemExt; }*/

        private:
            Common::RResult findFileImpl(const fs::path& path, const fs::path& fromPath, PathInfo& outPathInfo) const;

            // SearchDirectoryList* m_searchDirectories;
            std::shared_ptr<IRfxFileSystem> fileSystemExt_;
        };
    }
}