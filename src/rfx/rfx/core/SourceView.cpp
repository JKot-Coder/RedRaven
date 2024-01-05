#include "SourceView.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            UnownedStringSlice advanceBom(UnownedStringSlice stringSlice)
            {
                auto begin = stringSlice.begin();

                if (utf8::starts_with_bom(begin))
                    utf8::next(begin, stringSlice.end());

                return UnownedStringSlice(begin, stringSlice.end());
            }
        }

        /* !!!!!!!!!!!!!!!!!!!!!!!!!! PathInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
        const U8String PathInfo::getMostUniqueIdentity() const
        {
            switch (type)
            {
                case Type::Normal: return uniqueIdentity;
                case Type::FoundPath:
                case Type::FromString: return foundPath;
                default: return "";
            }
        }

        bool PathInfo::operator==(const PathInfo& rhs) const
        {
            // They must be the same type
            if (type != rhs.type)
                return false;

            switch (type)
            {
                case Type::TokenPaste:
                case Type::Unknown:
                case Type::CommandLine: return true;
                case Type::Normal: return foundPath == rhs.foundPath && uniqueIdentity == rhs.uniqueIdentity;
                case Type::FromString:
                case Type::FoundPath:
                    // Only have a found path
                    return foundPath == rhs.foundPath;
                default: break;
            }

            return false;
        }

        void SourceFile::SetContent(UnownedStringSlice content)
        {
            content_ = advanceBom(content);
        }

        bool extractLine(UnownedStringSlice& ioText, UnownedStringSlice& outLine)
        {
            U8Char const* const begin = ioText.begin();
            U8Char const* const end = ioText.end();

            if (begin == nullptr)
            {
                outLine = UnownedStringSlice(nullptr, nullptr);
                return false;
            }

            U8Char const* cursor = begin;
            while (cursor < end)
            {
                const auto ch = utf8::next(cursor, end);

                switch (ch)
                {
                    case '\r':
                    case '\n':
                    {
                        // Remember the end of the line
                        const U8Char* const lineEnd = cursor - 1;

                        // When we see a line-break character we need
                        // to record the line break, but we also need
                        // to deal with the annoying issue of encodings,
                        // where a multi-byte sequence might encode
                        // the line break.
                        if (cursor < end)
                        {
                            const auto d = *cursor;
                            if ((ch ^ d) == ('\r' ^ '\n'))
                                cursor++;
                        }

                        ioText = UnownedStringSlice(cursor, end);
                        outLine = UnownedStringSlice(begin, lineEnd);
                        return true;
                    }
                    default:
                        break;
                }
            }

            // There is nothing remaining
            ioText = UnownedStringSlice(nullptr, nullptr);

            // Could be empty, or the remaining line (without line end terminators of)
            ASSERT(begin <= cursor);

            outLine = UnownedStringSlice(begin, cursor);
            return true;
        }

        const U8Char* SourceView::GetContentFrom(const SourceLocation& loc) const
        {
            ASSERT(loc.GetSourceView() == shared_from_this());

            return GetContent().begin() + loc.raw;
        }

        UnownedStringSlice SourceView::ExtractLineContainingLocation(const SourceLocation& loc) const
        {
            ASSERT(loc.GetSourceView() == shared_from_this());

            const U8Char* const contentStart = GetContent().begin();
            const U8Char* const contentEnd = GetContent().end();
            const U8Char* pos = GetContentFrom(loc);

            // If we start with a newline character, we assume that we need a line before.
            for (; pos > contentStart; --pos)
            {
                // Work with UTF8 as ANSI text. This shouldn't be a problem...
                const auto ch = *pos;

                if (ch == '\n' || ch == '\r')
                    continue;

                break;
            }

            // We want to determine the start of the line, and the end of the line
            const U8Char* start = pos;
            for (; start > contentStart; --start)
            {
                // Work with UTF8 as ANSI text. This shouldn't be a problem...
                const auto ch = *start;
                if (ch == '\n' || ch == '\r')
                {
                    // We want the character after, but we can only do this if not already at pos
                    start += int(start < pos);
                    break;
                }
            }

            const U8Char* end = pos;
            for (; end < contentEnd; ++end)
            {
                const auto ch = *end;
                if (ch == '\n' || ch == '\r')
                    break;
            }

            return UnownedStringSlice(start, end);
        }
    }
}