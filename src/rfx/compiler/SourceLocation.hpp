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

#include "compiler/UnownedStringSlice.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
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
            public:
                /// To be more rigorous about where a path comes from, the type identifies what a paths origin is
                enum class Type : uint8_t
                {
                    Unknown, ///< The path is not known
                    Normal, ///< Normal has both path and uniqueIdentity
                };
                PathInfo() = default;
                PathInfo(U8String path) : type_(Type::Normal)
                {
                    pathStack.push_back(path);
                }

                const std::vector<U8String>& GetPathStack()
                {
                    return pathStack;
                }

            private:
                Type type_ = Type::Unknown; ///< The type of path
                std::vector<U8String> pathStack;
            };

            struct SourceLocation
            {
                using RawValue = size_t;

                SourceLocation() = default;
                SourceLocation(const SourceLocation& loc) : raw_(loc.raw_), sourceView_(loc.sourceView_) { }

                inline bool operator==(const SourceLocation& rhs) const { return (raw_ == rhs.raw_) && (sourceView_ == rhs.sourceView_); }
                inline bool operator!=(const SourceLocation& rhs) const { return (raw_ != rhs.raw_) && (sourceView_ != rhs.sourceView_); }

                inline SourceLocation& operator=(const SourceLocation& rhs) = default;

                inline std::shared_ptr<SourceView> GetSourceView() const { return sourceView_; }
                inline bool IsValid() const { return sourceView_ != nullptr; }

            private:
                friend SourceView;

                SourceLocation(RawValue raw, const std::shared_ptr<SourceView>& sourceView) : raw_(raw), sourceView_(sourceView)
                {
                    ASSERT(sourceView_)
                }

            private:
                RawValue raw_ = 0;
                std::shared_ptr<SourceView> sourceView_;
            };
            /*
            // A range of locations in the input source
            struct SourceRange
            {
            public:
                SourceRange() = default;
                SourceRange(SourceLocation loc) : begin(loc), end(loc) { }
                SourceRange(SourceLocation begin, SourceLocation end) : begin(begin), end(end) { ASSERT(end.GetRaw() > begin.GetRaw()); }

                /// True if the loc is in the range. Range is inclusive on begin to end.
                bool Contains(SourceLocation loc) const
                {
                    const auto rawLoc = loc.GetRaw();
                    return rawLoc >= begin.GetRaw() && rawLoc <= end.GetRaw();
                }

                /// Get the total size
                size_t GetSize() const { return uint32_t(end.GetRaw() - begin.GetRaw()); }

                /// Get the offset of a loc in this range
                size_t GetOffset(SourceLocation loc) const
                {
                    ASSERT(Contains(loc))
                    return (loc.GetRaw() - begin.GetRaw());
                }

                /// Convert an offset to a loc
                SourceLocation GetSourceLocationFromOffset(uint32_t offset) const
                {
                    ASSERT(offset <= GetSize());
                    return begin + int32_t(offset);
                }

                SourceLocation begin;
                SourceLocation end;
            };*/

            // A logical or physical storage object for a range of input code
            // that has logically contiguous source locations.
            class SourceFile final
            {
            public:
                struct OffsetRange
                {
                    /// We need a value to indicate an invalid range. We can't use 0 as that is valid for an offset range
                    /// We can't use a negative number, and don't want to make signed so we get the full 32-bits.
                    /// So we just use the max value as invalid
                    static const uint32_t kInvalid = 0xffffffff;

                    /// True if the range is valid
                    inline bool IsValid() const { return end >= start && start != kInvalid; }
                    /// True if offset is within range (inclusively)
                    inline bool ContainsInclusive(uint32_t offset) const { return offset >= start && offset <= end; }

                    /// Get the count
                    inline uint32_t GetCount() const { return end - start; }

                    /// Return an invalid range.
                    static OffsetRange MakeInvalid() { return OffsetRange { kInvalid, kInvalid }; }

                    uint32_t start;
                    uint32_t end;
                };

            public:
                //  SourceFile(SourceManager* sourceManager, const PathInfo& pathInfo, size_t contentSize);
                ~SourceFile() = default;

                /// Returns the line break offsets (in bytes from start of content)
                /// Note that this is lazily evaluated - the line breaks are only calculated on the first request
                const std::vector<size_t>& GetLineBreakOffsets();

                /// Calculate the line based on the offset
                uint32_t CalcLineIndexFromOffset(size_t offset);

                /// Calculate the offset for a line
                uint32_t CalcColumnIndex(uint32_t line, size_t offset);

                /// True if has full set content
                // bool HasContent() const { return contentBlob_ != nullptr; }

                /// Get the content size
                size_t GetContentSize() const { return contentSize_; }

                /// Get the content
                const UnownedStringSlice& GetContent() const { return content_; }

                /// Get path info
                // const PathInfo& getPathInfo() const { return m_pathInfo; }
                // Set the content as a string
                void SetContents(const U8String& content);

                /// Calculate a display path -> can canonicalize if necessary
                U8String CalcVerbosePath() const;

                /// Get the source manager this was created on
                // SourceManager* GetSourceManager() const { return m_sourceManager; }

            private:
                // SourceManager* sourceManager_; ///< The source manager this belongs to
                // PathInfo pathInfo_; ///< The path The logical file path to report for locations inside this span.
                // ComPtr<ISlangBlob> contentBlob_; ///< A blob that owns the storage for the file contents. If nullptr, there is no contents
                UnownedStringSlice content_; ///< The actual contents of the file.
                size_t contentSize_; ///< The size of the actual contents

                // In order to speed up lookup of line number information,
                // we will cache the starting offset of each line break in
                // the input file:
                std::vector<size_t> lineBreakOffsets_;
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
                HumaneSourceLocation(uint32_t line, uint32_t column, const PathInfo& pathInfo)
                    : line(line), column(column), pathInfo(pathInfo) {};

                uint32_t line = 0;
                uint32_t column = 0;
                PathInfo pathInfo;
            };

            /* A SourceView maps to a single span of SourceLocation range and is equivalent to a single include or more precisely use of a source file.
            It is distinct from a SourceFile - because a SourceFile may be included multiple times, with different interpretations (depending
            on #defines for example).
            */
            class SourceView final : public std::enable_shared_from_this<SourceView>
            {
            public:
                /// Get the source file holds the contents this view
                std::shared_ptr<SourceFile> GetSourceFile() const { return sourceFile_; }
                /// Get the source manager
                // SourceManager* GetSourceManager() const { return sourceFile_->getSourceManager(); }

                /// Get the associated 'content' (the source text)
                const UnownedStringSlice& GetContent() const { return sourceFile_->GetContent(); }

                const U8Char* GetContentFrom(const SourceLocation& loc) const;

                /// Gets the pathInfo for this view. It may be different from the m_sourceFile's if the path has been
                /// overridden by m_viewPath
                PathInfo getPathInfo() const { return pathInfo_; }

                /// Get the size of the content
                size_t GetContentSize() const { return sourceFile_->GetContentSize(); }

                SourceLocation GetSourceLocation(size_t offset);

                /// Get the humane location
                /// Type determines if the location wanted is the original, or the 'normal' (which modifys behavior based on #line directives)
                HumaneSourceLocation GetHumaneLocation(const SourceLocation& loc, SourceLocationType type = SourceLocationType::Nominal);

                UnownedStringSlice ExtractLineContainingLocation(const SourceLocation& loc);

            public:
                [[nodiscard]] static std::shared_ptr<SourceView> Create(const std::shared_ptr<SourceFile>& sourceFile, const PathInfo& pathInfo)
                {
                    return std::shared_ptr<SourceView>(new SourceView(sourceFile, pathInfo));
                }

            private:
                SourceView(const std::shared_ptr<SourceFile>& sourceFile, const PathInfo& pathInfo)
                    : sourceFile_(sourceFile),
                      pathInfo_(pathInfo)
                {
                }

                std::shared_ptr<SourceFile> sourceFile_; ///< The source file. Can hold the line breaks
                PathInfo pathInfo_; ///< Path to this view. If empty the path is the path to the SourceView
                SourceLocation initiatingSourceLocation_; ///< An optional source loc that defines where this view was initiated from. SourceLocation(0) if not defined.
            };
        }
    }
}