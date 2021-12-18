#include "SourceLocation.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            void SourceFile::SetContents(const U8String& content)
            {
                contentSize_ = content.length();
                content_ = UnownedStringSlice(content);
            }

            bool extractLine(UnownedStringSlice& ioText, UnownedStringSlice& outLine)
            {
                U8Char const* const begin = ioText.Begin();
                U8Char const* const end = ioText.End();

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

            const std::vector<size_t>& SourceFile::GetLineBreakOffsets()
            {
                // We now have a raw input file that we can search for line breaks.
                // We obviously don't want to do a linear scan over and over, so we will
                // cache an array of line break locations in the file.
                if (lineBreakOffsets_.empty())
                {
                    UnownedStringSlice content(GetContent()), line;
                    const U8Char* contentBegin = content.Begin();

                    while (extractLine(content, line))
                    {
                        lineBreakOffsets_.push_back(std::distance(contentBegin, line.Begin()));
                    }
                    // Note that we do *not* treat the end of the file as a line
                    // break, because otherwise we would report errors like
                    // "end of file inside string literal" with a line number
                    // that points at a line that doesn't exist.
                }

                return lineBreakOffsets_;
            }

            uint32_t SourceFile::CalcLineIndexFromOffset(size_t offset)
            {
                ASSERT(offset <= GetContentSize());

                // Make sure we have the line break offsets
                const auto& lineBreakOffsets = GetLineBreakOffsets();

                // At this point we can assume the `lineBreakOffsets` array has been filled in.
                // We will use a binary search to find the line index that contains our
                // chosen offset.
                size_t lo = 0;
                size_t hi = lineBreakOffsets.size();

                while (lo + 1 < hi)
                {
                    const uint32_t mid = (hi + lo) >> 1;
                    const uint32_t midOffset = lineBreakOffsets[mid];
                    if (midOffset <= uint32_t(offset))
                    {
                        lo = mid;
                    }
                    else
                    {
                        hi = mid;
                    }
                }

                return int(lo);
            }

            /// Calculate the offset for a line
            uint32_t SourceFile::CalcColumnIndex(uint32_t line, size_t offset)
            {
                const auto& lineBreakOffsets = GetLineBreakOffsets();
                ASSERT(line < lineBreakOffsets.size())
                const auto lineBegin = content_.Begin() + lineBreakOffsets[line];

                return utf8::distance(lineBegin, content_.Begin() + offset);
            }

            const U8Char* SourceView::GetContentFrom(const SourceLocation& loc) const
            {
                ASSERT(loc.sourceView_ == shared_from_this());

                return GetContent().Begin() + loc.raw_;
            }

            SourceLocation SourceView::GetSourceLocation(size_t offset)
            {
                ASSERT(offset <= GetContentSize());
                return SourceLocation(offset, shared_from_this());
            }

            HumaneSourceLocation SourceView::GetHumaneLocation(const SourceLocation& loc, SourceLocationType type)
            {
                ASSERT(loc.sourceView_ == shared_from_this());

                (void)type;

                const auto offset = loc.raw_;

                // We need the line index from the original source file
                const auto lineIndex = sourceFile_->CalcLineIndexFromOffset(offset);

                // TODO: we should really translate the byte index in the line
                // to deal with:
                // - Tab characters, which should really adjust how we report
                //   columns (although how are we supposed to know the setting
                //   that an IDE expects us to use when reporting locations?)
                const auto columnIndex = sourceFile_->CalcColumnIndex(lineIndex, offset);

                HumaneSourceLocation humaneLoc;
                humaneLoc.column = columnIndex + 1;
                humaneLoc.line = lineIndex + 1;

                /*
                // Make up a default entry
                StringSlicePool::Handle pathHandle = StringSlicePool::Handle(0);

                // Only bother looking up the entry information if we want a 'Normal' lookup
                const int entryIndex = (type == SourceLocType::Nominal) ? findEntryIndex(loc) : -1;
                if (entryIndex >= 0)
                {
                    const Entry& entry = m_entries[entryIndex];
                    // Adjust the line
                    humaneLoc.line += entry.m_lineAdjust;
                    // Get the pathHandle..
                    pathHandle = entry.m_pathHandle;
                }

                humaneLoc.pathInfo = _getPathInfoFromHandle(pathHandle);*/
                return humaneLoc;
            }

            UnownedStringSlice SourceView::ExtractLineContainingLocation(const SourceLocation& loc)
            {
                ASSERT(loc.sourceView_ == shared_from_this());

                const U8Char* const contentStart = GetContent().Begin();
                const U8Char* const contentEnd = GetContent().End();
                const U8Char* const pos = GetContentFrom(loc);

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
}