#pragma once

/** Overview:

There needs to be a mechanism where we can easily and quickly track a specific locations in any source file used during a compilation.
This is important because that original location is meaningful to the user as it relates to their original source. Thus SourceLocation are
used so we can display meaningful and accurate errors/warnings as well as being able to always map generated code locations back to their origins.

A 'SourceLocation' along with associated structures (SourceView, SourceFile, SourceMangager) this can pinpoint the location down to the byte across the
compilation. This could be achieved by storing for every token and instruction the file, line and column number came from. The SourceLocation is used in
lots of places - every AST node, every Token from the lexer, every IRInst - so we really want to make it small. So for this reason we actually
encode SourceLocation as a single integer and then use the associated structures when needed to determine what the location actually refers to -
the source file, line and column number, or in effect the byte in the original file.

Unfortunately there is extra complications. When a source is parsed it's interpretation (in terms of how a piece of source maps to an 'original' file etc)
can be overridden - for example by using #line directives. Moreover a single source file can be parsed multiple times. When it's parsed multiple times the
interpretation of the mapping (#line directives for example) can change. This is the purpose of the SourceView - it holds the interpretation of a source file
for a specific Lex/Parse.

Another complication is that not all 'source' comes from SourceFiles, a macro expansion, may generate new 'source' we need to handle this, but also be able
to have a SourceLocation map to the expansion unambiguously. This is handled by creating a SourceFile and SourceView that holds only the macro generated
specific information.

SourceFile - Is the immutable text contents of a file (or perhaps some generated source - say from doing a macro substitution)
SourceView - Tracks a single parse of a SourceFile. Each SourceView defines a range of source locations used. If a SourceFile is parsed twice, two
SourceViews are created, with unique SourceRanges. This is so that it is possible to tell which specific parse a SourceLocation is from - and so know the right
interpretation for that lex/parse.
*/

#include "rfx/core/UnownedStringSlice.hpp"

#include <sstream>

namespace RR
{
    namespace Rfx
    {
        class SourceView;

        /** Overview:

            TODO Rewrite this comment

            There needs to be a mechanism where we can easily and quickly track a specific locations in any source file used during a compilation.
            This is important because that original location is meaningful to the user as it relates to their original source. Thus SourceLoc are
            used so we can display meaningful and accurate errors/warnings as well as being able to always map generated code locations back to their origins.

            A 'SourceLoc' along with associated structures (SourceView, SourceFile, SourceMangager) this can pinpoint the location down to the byte across the
            compilation. This could be achieved by storing for every token and instruction the file, line and column number came from. The SourceLoc is used in
            lots of places - every AST node, every Token from the lexer, every IRInst - so we really want to make it small. So for this reason we actually
            encode SourceLoc as a single integer and then use the associated structures when needed to determine what the location actually refers to -
            the source file, line and column number, or in effect the byte in the original file.

            Unfortunately there is extra complications. When a source is parsed it's interpretation (in terms of how a piece of source maps to an 'original' file etc)
            can be overridden - for example by using #line directives. Moreover a single source file can be parsed multiple times. When it's parsed multiple times the
            interpretation of the mapping (#line directives for example) can change. This is the purpose of the SourceView - it holds the interpretation of a source file
            for a specific Lex/Parse.

            Another complication is that not all 'source' comes from SourceFiles, a macro expansion, may generate new 'source' we need to handle this, but also be able
            to have a SourceLoc map to the expansion unambiguously. This is handled by creating a SourceFile and SourceView that holds only the macro generated
            specific information.

            SourceFile - Is the immutable text contents of a file (or perhaps some generated source - say from doing a macro substitution)
            SourceView - Tracks a single parse of a SourceFile. Each SourceView defines a range of source locations used. If a SourceFile is parsed twice, two
            SourceViews are created, with unique SourceRanges. This is so that it is possible to tell which specific parse a SourceLoc is from - and so know the right
            interpretation for that lex/parse.
            */

        struct PathInfo
        {
            typedef PathInfo ThisType;

            /// To be more rigorous about where a path comes from, the type identifies what a paths origin is
            enum class Type : uint8_t
            {
                Unknown, ///< The path is not known
                Normal, ///< Normal has both path and uniqueIdentity
                FoundPath, ///< Just has a found path (uniqueIdentity is unknown, or even 'unknowable')
                FromString, ///< Created from a string (so found path might not be defined and should not be taken as to map to a loaded file)
                TokenPaste, ///< No paths, just created to do a macro expansion
                TypeParse, ///< No path, just created to do a type parse
                CommandLine, ///< A macro constructed from the command line

                Split,
            };

            /// True if has a canonical path
            inline bool hasUniqueIdentity() const { return type == Type::Normal && uniqueIdentity.length() > 0; }
            /// True if has a regular found path
            inline bool hasFoundPath() const { return type == Type::Normal || type == Type::FoundPath || (type == Type::FromString && foundPath.length() > 0); }
            /// True if has a found path that has originated from a file (as opposed to string or some other origin)
            inline bool hasFileFoundPath() const { return (type == Type::Normal || type == Type::FoundPath) && foundPath.length() > 0; }

            bool operator==(const ThisType& rhs) const;
            bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Returns the 'most unique' identity for the path. If has a 'uniqueIdentity' returns that, else the foundPath, else "".
            const U8String getMostUniqueIdentity() const;

            static PathInfo makeSplit(const U8String& foundPathIn, const U8String& uniqueIdentity)
            {
                ASSERT(uniqueIdentity.length() > 0 && foundPathIn.length() > 0)
                return PathInfo { Type::Split, foundPathIn, uniqueIdentity };
            }

            // So simplify construction. In normal usage it's safer to use make methods over constructing directly.
            static PathInfo makeUnknown() { return PathInfo { Type::Unknown, U8String(), U8String() }; }
            static PathInfo makeTokenPaste() { return PathInfo { Type::TokenPaste, "token paste", U8String() }; }
            static PathInfo makeNormal(const U8String& foundPathIn, const U8String& uniqueIdentity)
            {
                ASSERT(uniqueIdentity.length() > 0 && foundPathIn.length() > 0)
                return PathInfo { Type::Normal, foundPathIn, uniqueIdentity };
            }
            static PathInfo makePath(const U8String& pathIn)
            {
                ASSERT(pathIn.length() > 0)
                return PathInfo { Type::FoundPath, pathIn, U8String() };
            }
            static PathInfo makeTypeParse() { return PathInfo { Type::TypeParse, "type string", U8String() }; }
            static PathInfo makeCommandLine() { return PathInfo { Type::CommandLine, "command line", U8String() }; }
            static PathInfo makeFromString(const U8String& userPath) { return PathInfo { Type::FromString, userPath, U8String() }; }

            Type type; ///< The type of path
            U8String foundPath; ///< The path where the file was found (might contain relative elements)
            U8String uniqueIdentity; ///< The unique identity of the file on the path found
        };

        // A logical or physical storage object for a range of input code
        // that has logically contiguous source locations.
        class SourceFile final
        {
        public:
            SourceFile(const PathInfo& pathInfo) : pathInfo_(pathInfo) {};
            ~SourceFile() = default;

            UnownedStringSlice GetContent() const { return content_; }
            size_t GetContentSize() const { return content_.GetLength(); }
            const PathInfo& GetPathInfo() const { return pathInfo_; }
            void SetContent(const UnownedStringSlice& content);

            /// Calculate a display path -> can canonicalize if necessary
            U8String CalcVerbosePath() const;

        private:
            UnownedStringSlice content_; ///< The actual contents of the file.
            PathInfo pathInfo_; ///< The path The logical file path to report for locations inside this span.
        };

        enum class SourceLocationType
        {
            Nominal, ///< The normal interpretation which takes into account #line directives
            Actual, ///< Ignores #line directives - and is the location as seen in the actual file
        };

        // A source location in a format a human might like to see
        struct HumaneSourceLocation
        {
            HumaneSourceLocation() = default;
            HumaneSourceLocation(uint32_t line, uint32_t column)
                : line(line), column(column) {};

            inline bool operator==(const HumaneSourceLocation& rhs) const { return (line == rhs.line) && (column == rhs.column); }
            inline bool operator!=(const HumaneSourceLocation& rhs) const { return (line != rhs.line) || (column != rhs.column); }
            inline HumaneSourceLocation& operator=(const HumaneSourceLocation& rhs) = default;

            uint32_t line = 0;
            uint32_t column = 0;
        };
        /*
        struct IncludeStack
        {
        public:
            struct IncludeInfo
            {
                PathInfo pathInfo;
                HumaneSourceLocation humaneSourceLocation;
            };

            IncludeStack() = default;
            IncludeStack(const PathInfo& pathInfo)
            {
                stack_.push_back({ pathInfo, HumaneSourceLocation() });
            }

            PathInfo GetOwnPathInfo() const
            {
                if (stack_.empty())
                    return PathInfo::makeUnknown();

                return stack_.back().pathInfo;
            }

            const std::vector<IncludeInfo>& GetStack() { return stack_; }

            bool IsValid() const { return !stack_.empty(); }

            static IncludeStack CreateIncluded(const IncludeStack& parentStack, const PathInfo& ownPathInfo, const HumaneSourceLocation& includeLocationInParent)
            {
                ASSERT(parentStack.IsValid())

                IncludeStack includeStack(parentStack);
                // Update parent include info with human source
                includeStack.stack_.back().humaneSourceLocation = includeLocationInParent;
                includeStack.stack_.push_back({ ownPathInfo, HumaneSourceLocation() });

                return includeStack;
            }

            static IncludeStack CreateSplitted(const IncludeStack& parentStack, const PathInfo& ownPathInfo)
            {
                ASSERT(parentStack.IsValid())

                IncludeStack includeStack(parentStack);
                // Replace parent include
                includeStack.stack_.pop_back();
                includeStack.stack_.push_back({ ownPathInfo, HumaneSourceLocation() });

                return includeStack;
            }

        private:
            std::vector<IncludeInfo> stack_;
        };
        */
        struct SourceLocation
        {
            using RawValue = size_t;

            SourceLocation() = default;
            SourceLocation(const SourceLocation& loc) : raw(loc.raw), sourceView(loc.sourceView), humaneSourceLoc(loc.humaneSourceLoc) { }

            inline bool operator==(const SourceLocation& rhs) const { return (raw == rhs.raw) && (humaneSourceLoc == rhs.humaneSourceLoc) && (sourceView.lock() == rhs.sourceView.lock()); }
            inline bool operator!=(const SourceLocation& rhs) const { return (raw != rhs.raw) || (humaneSourceLoc != rhs.humaneSourceLoc) || (sourceView.lock() != rhs.sourceView.lock()); }

            inline SourceLocation& operator=(const SourceLocation& rhs) = default;

            inline std::shared_ptr<const SourceView> GetSourceView() const { return sourceView.lock(); }

        private:
            friend SourceView;

            SourceLocation(RawValue raw, const HumaneSourceLocation& humaneSourceLocation, const std::shared_ptr<const SourceView>& sourceView)
                : raw(raw), humaneSourceLoc(humaneSourceLocation), sourceView(sourceView)
            {
                ASSERT(sourceView);
            }

        public:
            RawValue raw = 0;
            HumaneSourceLocation humaneSourceLoc;
            std::weak_ptr<const SourceView> sourceView;
        };

        /* A SourceView maps to a single span of SourceLocation range and is equivalent to a single include or more precisely use of a source file.
            It is distinct from a SourceFile - because a SourceFile may be included multiple times, with different interpretations (depending
            on #defines for example).
            */
        class SourceView final : public std::enable_shared_from_this<SourceView>, Common::NonCopyable
        {
        public:
            /// Get the source file holds the contents this view
            std::shared_ptr<SourceFile> GetSourceFile() const { return sourceFile_.lock(); }

            /// Get the associated 'content' (the source text)
            UnownedStringSlice GetContent() const { return content_; }

            const U8Char* GetContentFrom(const SourceLocation& loc) const;

            /// Gets the pathInfo for this view. It may be different from the m_sourceFile's if the path has been
            /// overridden by m_viewPath TODO COMMENT
            // IncludeStack GetIncludeStack() const { return includeStack_; }

            size_t GetContentSize() const { return content_.GetLength(); }

            SourceLocation GetSourceLocation(size_t offset, const HumaneSourceLocation& humaneSourceLoc) const
            {
                ASSERT(offset <= GetContentSize());
                return SourceLocation(offset, humaneSourceLoc, shared_from_this());
            }

            /// Gets the pathInfo for this view. It may differ from the pathInfo of the source file if the sourceView was created by #line directive.
            PathInfo GetPathInfo() const { return pathInfo_; }

            /// Get the humane location
            /// Type determines if the location wanted is the original, or the 'normal' (which modifys behavior based on #line directives)
            // HumaneSourceLocation GetHumaneLocation(const SourceLocation& loc, SourceLocationType type = SourceLocationType::Nominal)

            SourceLocation GetInitiatingSourceLocation() const { return initiatingSourceLocation_; }
            UnownedStringSlice ExtractLineContainingLocation(const SourceLocation& loc) const;

        public:
            [[nodiscard]]
            static std::shared_ptr<SourceView> CreateFromSourceFile(const std::shared_ptr<SourceFile>& sourceFile)
            {
                auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), sourceFile->GetPathInfo(), {}));
                sourceView->initiatingSourceLocation_ = SourceLocation(0, HumaneSourceLocation(1, 1), sourceView);
                sourceView->advanceBom();

                return sourceView;
            }

            [[nodiscard]]
            static std::shared_ptr<SourceView> CreatePasted(const std::shared_ptr<SourceFile>& sourceFile,
                                                            const SourceLocation& parentSourceLocation)
            {
                ASSERT(sourceFile);

                const auto& pathInfo = PathInfo::makeTokenPaste();
                auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), pathInfo, parentSourceLocation));
                sourceView->advanceBom();

                return sourceView;
            }

            [[nodiscard]]
            static std::shared_ptr<SourceView> CreateIncluded(const std::shared_ptr<SourceFile>& sourceFile,
                                                              const SourceLocation& parentSourceLocation)
            {
                ASSERT(sourceFile);

                const auto& pathInfo = sourceFile->GetPathInfo();
                auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), pathInfo, parentSourceLocation));
                sourceView->advanceBom();

                return sourceView;
            }

            [[nodiscard]]
            static std::shared_ptr<SourceView> CreateSplited(const SourceLocation& splitLocation, const PathInfo& ownPathInfo)
            {
                const auto sourceView = splitLocation.GetSourceView();
                ASSERT(sourceView);

                const auto content = UnownedStringSlice(sourceView->GetContentFrom(splitLocation), sourceView->GetSourceFile()->GetContent().End());
                return std::shared_ptr<SourceView>(new SourceView(sourceView->GetSourceFile(), content, ownPathInfo, splitLocation));
            }

        private:
            SourceView(const std::shared_ptr<SourceFile>& sourceFile,
                       const UnownedStringSlice& content,
                       const PathInfo& pathInfo,
                       const SourceLocation& initiatingSourceLocation)
                : sourceFile_(sourceFile), content_(content), pathInfo_(pathInfo), initiatingSourceLocation_(initiatingSourceLocation)
            {
                ASSERT(sourceFile); // TODO validate file
            }

            void advanceBom()
            {
                auto begin = content_.Begin();

                if (utf8::starts_with_bom(begin))
                    utf8::next(begin, content_.End());

                content_ = UnownedStringSlice(begin, content_.End());
            }

            std::weak_ptr<SourceFile> sourceFile_; ///< The source file. Can hold the line breaks
            // IncludeStack includeStack_; ///< Path to this view. If empty the path is the path to the SourceView
            PathInfo pathInfo_;
            UnownedStringSlice content_;
            SourceLocation initiatingSourceLocation_; ///< An optional source loc that defines where this view was initiated from. SourceLocation(0) if not defined.
        };
    }
}