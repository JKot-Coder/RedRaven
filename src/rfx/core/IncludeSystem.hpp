#pragma once

namespace RR
{
    namespace Rfx
    {
        struct PathInfo;
        class IRfxFileSystemExt;

        enum class PathType : uint32_t
        {
            Directory, /**< Path specified specifies a directory. */
            File, /**< Path specified is to a file. */
        };

        /* A helper class that builds basic include handling on top of searchDirectories/fileSystemExt */ //TODO
        class IncludeSystem final
        {
        public:
            IncludeSystem(const std::shared_ptr<IRfxFileSystemExt>& fileSystemExt)
                : fileSystemExt_(fileSystemExt) {};

            /* IncludeSystem(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt)
                    : m_searchDirectories(searchDirectories),
                      m_fileSystemExt(fileSystemExt)
                {
                }*/

            RfxResult FindFile(const U8String& pathToInclude, const U8String& pathIncludedFrom, PathInfo& outPathInfo);
            RfxResult FindFile(PathType fromPathType, const U8String& fromPath, const U8String& path, PathInfo& outPathInfo);
            /*String simplifyPath(const String& path);
                SlangResult loadFile(const PathInfo& pathInfo, ComPtr<ISlangBlob>& outBlob);

                SlangResult findAndLoadFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo, ComPtr<ISlangBlob>& outBlob);

                SearchDirectoryList* getSearchDirectoryList() const { return m_searchDirectories; }
                ISlangFileSystemExt* getFileSystem() const { return m_fileSystemExt; }*/
        private:
            // SearchDirectoryList* m_searchDirectories;
            std::shared_ptr<IRfxFileSystemExt> fileSystemExt_;
        };
    }
}