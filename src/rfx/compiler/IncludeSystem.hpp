#pragma once

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            /* A helper class that builds basic include handling on top of searchDirectories/fileSystemExt */ //TODO
            class IncludeSystem final
            {
                /* IncludeSystem(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt)
                    : m_searchDirectories(searchDirectories),
                      m_fileSystemExt(fileSystemExt)
                {
                }

                SlangResult findFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo);
                SlangResult findFile(SlangPathType fromPathType, const String& fromPath, const String& path, PathInfo& outPathInfo);
                String simplifyPath(const String& path);
                SlangResult loadFile(const PathInfo& pathInfo, ComPtr<ISlangBlob>& outBlob);

                SlangResult findAndLoadFile(const String& pathToInclude, const String& pathIncludedFrom, PathInfo& outPathInfo, ComPtr<ISlangBlob>& outBlob);

                SearchDirectoryList* getSearchDirectoryList() const { return m_searchDirectories; }
                ISlangFileSystemExt* getFileSystem() const { return m_fileSystemExt; }*/

            public:
                [[nodiscard]] static std::shared_ptr<IncludeSystem> Create()
                {
                    return std::shared_ptr<IncludeSystem>(new IncludeSystem());
                }

            private:
                IncludeSystem() {};

                // SearchDirectoryList* m_searchDirectories;
                //ISlangFileSystemExt* m_fileSystemExt;
            };
        }
    }
}