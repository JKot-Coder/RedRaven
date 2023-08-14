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

            uint32_t line = 0;
            uint32_t column = 0;
        };
        struct SourceLocation
        {
            using RawValue = size_t;

            SourceLocation() = default;
            SourceLocation(const SourceLocation& loc) : raw(loc.raw), humaneSourceLoc(loc.humaneSourceLoc), sourceView(loc.sourceView) { }

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
    }
}