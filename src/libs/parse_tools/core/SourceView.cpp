#include "SourceView.hpp"

namespace RR
{
    namespace ParseTools
    {
        namespace
        {
            UnownedStringSlice advanceBom(UnownedStringSlice stringSlice)
            {
                auto begin = stringSlice.begin();
                const auto end = stringSlice.end();

                if (utf8::starts_with_bom(begin, end))
                    utf8::next(begin, end);

                return UnownedStringSlice(begin, end);
            }
        }

        /* !!!!!!!!!!!!!!!!!!!!!!!!!! PathInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
        const std::string PathInfo::getMostUniqueIdentity() const
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
            const auto begin = ioText.begin();
            const auto end = ioText.end();

            if (&*begin == nullptr)
            {
                outLine = UnownedStringSlice {};
                return false;
            }

            auto cursor = begin;
            while (cursor < end)
            {
                const auto prev = cursor;
                const auto ch = utf8::next(cursor, end);

                switch (ch)
                {
                    case '\r':
                    case '\n':
                    {
                        // Remember the end of the line
                        const auto lineEnd = prev;

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
            ioText = UnownedStringSlice {};

            // Could be empty, or the remaining line (without line end terminators of)
            ASSERT(begin <= cursor);

            outLine = UnownedStringSlice(begin, cursor);
            return true;
        }

        UnownedStringSlice::const_iterator SourceView::GetContentFrom(const SourceLocation& loc) const
        {
            ASSERT(loc.GetSourceView() == shared_from_this());

            return GetContent().begin() + loc.raw;
        }

        UnownedStringSlice SourceView::ExtractLineContainingLocation(const SourceLocation& loc) const
        {
            ASSERT(loc.GetSourceView() == shared_from_this());

            const auto contentStart = GetContent().begin();
            const auto contentEnd = GetContent().end();
            auto pos = GetContentFrom(loc);

            if (pos == contentEnd)
                --pos;

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
            auto start = pos;
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

            auto end = pos;
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