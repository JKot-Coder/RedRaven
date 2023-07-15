#include "DiagnosticSink.hpp"

#include "rfx/compiler/Lexer.hpp"
#include "rfx/compiler/Signal.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            /// Trims any horizontal whitespace from the start and end and returns as a substring
            UnownedStringSlice trim(const UnownedStringSlice& stringSlice)
            {
                const U8Char* start = stringSlice.Begin();
                const U8Char* end = stringSlice.End();

                // Work with UTF8 as ANSI text. This shouldn't be a problem...
                while (start < end && (*start == '\t' || *start == ' '))
                    start++;
                while (end > start && (*start == '\t' || *start == ' '))
                    end--;

                return UnownedStringSlice(start, end);
            }

            U8String replaceTabWithSpaces(const UnownedStringSlice& slice, uint32_t tabSize)
            {
                const U8Char* start = slice.Begin();
                const U8Char* const end = slice.End();
                U8String out;

                for (const U8Char* cur = start; cur < end;)
                {
                    const auto ch = utf8::next(cur, end);

                    if (ch == '\t')
                    {
                        if (start < cur)
                            out.append(start, cur - 1);

                        // Add the spaces
                        out.append(tabSize, ' ');

                        // Set the start at the first character past
                        start = cur;
                    }
                }

                if (start < end)
                {
                    out.append(start, end);
                }

                return out;
            }

            U8String reduceLengthFromHead(size_t startIndex, const U8String& string)
            {
                auto head = utf8::iterator<U8String::const_iterator>(string.begin(), string.begin(), string.end());

                while (startIndex--)
                    head++;

                return U8String(head.base(), string.end());
            }

            U8String reduceLengthFromTail(size_t maxLength, const U8String& string)
            {
                auto tail = utf8::iterator<U8String::const_iterator>(string.end(), string.begin(), string.end());

                size_t length = utf8::distance(string.begin(), string.end());
                ASSERT(length > maxLength)

                size_t reduceLength = length - maxLength;

                while (reduceLength--)
                    tail--;

                return U8String(string.begin(), tail.base());
            }

            U8String sourceLocationNoteDiagnostic(const Diagnostic& diagnostic, size_t maxLineLength)
            {
                ASSERT(diagnostic.location.IsValid())

                const auto sourceView = diagnostic.location.GetSourceView();

                UnownedStringSlice sourceLineSlice = sourceView->ExtractLineContainingLocation(diagnostic.location);

                // Trim any trailing white space
                sourceLineSlice = UnownedStringSlice(sourceLineSlice.Begin(), trim(sourceLineSlice).End());

                // TODO(JS): The tab size should ideally be configurable from command line.
                // For now just go with 4.
                const uint32_t tabSize = 4;

                U8String lineToLocation = U8String(sourceLineSlice.Begin(), sourceView->GetContentFrom(diagnostic.location));
                lineToLocation = replaceTabWithSpaces(lineToLocation, tabSize);
                size_t caretOffset = utf8::distance(lineToLocation.begin(), lineToLocation.end());

                auto sourceLine = replaceTabWithSpaces(sourceLineSlice, tabSize);
                U8String caretLine;

                // Now the caretLine which appears underneath the sourceLine
                {
                    // Append spaces as many
                    caretLine.append(caretOffset, ' ');

                    // Add caret
                    caretLine += "^";

                    if (diagnostic.isTokenValid)
                    {
                        const auto length = diagnostic.stringSlice.GetLength();

                        if (length > 1)
                            caretLine.append(length - 1, '~');
                    }

                    if (maxLineLength > 0)
                    {
                        const U8String ellipsis = "...";
                        const U8String spaces = "   ";
                        ASSERT(ellipsis.length() == spaces.length())
                        const auto ellipsisLen = ellipsis.length();

                        // We use the caretLine length if we have a token, because it will have underscores such that it's end is the end of
                        // the item at issue.
                        // If we don't have the token, we guesstimate using 1/4 of the maximum length
                        const auto endIndex = diagnostic.isTokenValid ? caretLine.length() : (caretOffset + (maxLineLength / 4));

                        if (endIndex > maxLineLength)
                        {
                            const auto startIndex = endIndex - (maxLineLength - ellipsisLen);

                            sourceLine = ellipsis + reduceLengthFromHead(startIndex, sourceLine);
                            caretLine = spaces + reduceLengthFromHead(startIndex, caretLine);
                        }

                        if (size_t(utf8::distance(sourceLine.begin(), sourceLine.end())) > maxLineLength)
                            sourceLine = ellipsis + reduceLengthFromTail(maxLineLength - ellipsisLen,
                                                                         U8String(sourceLine.begin() + ellipsisLen, sourceLine.end()));
                    }
                }

                // We could have handling here for if the line is too long, that we surround the important section
                // will ellipsis for example.
                // For now we just output.
                return fmt::format("{0}\n{1}\n", sourceLine, caretLine);
            }
        }

        U8String GetSeverityName(Severity severity)
        {
            switch (severity)
            {
                case Severity::Note: return "note";
                case Severity::Warning: return "warning";
                case Severity::Error: return "error";
                case Severity::Fatal: return "fatal error";
                case Severity::Internal: return "internal error";
                default: return "unknown error";
            }
        }

        void DiagnosticSink::diagnoseImpl(const DiagnosticInfo& info, const U8String& formattedMessage)
        {
            if (info.severity >= Severity::Error)
                errorCount_++;

            for (const auto& writer : writerList_)
                writer->Write(formattedMessage);

            // TODO replace
            if (info.severity > Severity::Internal)
            {
                // TODO: figure out a better policy for aborting compilation
                RFX_ABORT_COMPILATION("");
            }
        }

        U8String DiagnosticSink::formatDiagnostic(const Diagnostic& diagnostic)
        {
            ASSERT(diagnostic.location.IsValid())

            U8String humaneLocString;
            {
                HumaneSourceLocation humaneLocation = diagnostic.humaneSourceLocation;
                const auto sourceView = diagnostic.location.GetSourceView();

                /*
                    CLion fomat
                    humaneLocString = fmt::format("{0}:{1}:{2}: ",
                                                  sourceView->GetSourceFile()->GetFileName(),
                                                  humaneLocation.line,
                                                  humaneLocation.column);
                */
                /*
                const auto includeStack = sourceView->GetIncludeStack().GetStack();
                ASSERT(!includeStack.empty())

                for (auto it = includeStack.begin(); it != std::prev(includeStack.end()); ++it)
                {
                    const auto& includeInfo = *it;
                    humaneLocString += fmt::format("In file included from {0}({1}):\n",
                                                   includeInfo.pathInfo.foundPath,
                                                   includeInfo.humaneSourceLocation.line,
                                                   includeInfo.humaneSourceLocation.column);
                }*/
                const auto path = onlyRelativePaths_ ? sourceView->GetPathInfo().foundPath : sourceView->GetPathInfo().uniqueIdentity;
                humaneLocString += fmt::format("{0}({1}): ",
                                               path,
                                               humaneLocation.line,
                                               humaneLocation.column);
            }

            U8String format = (diagnostic.errorID >= 0) ? "{0} {1}: {2}\n" : "{0}: {2}\n";

            U8String diagnosticString = humaneLocString + fmt::format(format,
                                                                      GetSeverityName(diagnostic.severity),
                                                                      diagnostic.errorID,
                                                                      diagnostic.message);

            // We don't don't output source line information if this is a 'note' as a note is extra information for one
            // of the other main severity types, and so the information should already be output on the initial line
            // if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::SourceLocationLine) && diagnostic.severity != Severity::Note)
            {
                diagnosticString += sourceLocationNoteDiagnostic(diagnostic, GetSourceLineMaxLength());
            }

            return diagnosticString;
        }
    }
}