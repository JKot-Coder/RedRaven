#include "DiagnosticSink.hpp"

#include "parse_tools/DiagnosticCore.hpp"
#include "parse_tools/Lexer.hpp"
#include "parse_tools/core/Signal.hpp"
#include "parse_tools/core/SourceView.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            /// Trims any horizontal whitespace from the start and end and returns as a substring
            UnownedStringSlice trim(UnownedStringSlice stringSlice)
            {
                auto start = stringSlice.begin();
                auto end = stringSlice.end();

                // Work with UTF8 as ANSI text. This shouldn't be a problem...
                while (start < end && (*start == '\t' || *start == ' '))
                    start++;
                while (end > start && (*start == '\t' || *start == ' '))
                    end--;

                return UnownedStringSlice(start, end);
            }

            template <typename T>
            std::string replaceTabWithSpaces(T string, uint32_t tabSize)
            {
                auto start = string.begin();
                const auto end = string.end();
                std::string out;

                for (auto cur = start; cur < end;)
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

            std::string reduceLengthFromHead(size_t startIndex, const std::string& string)
            {
                auto head = utf8::iterator<std::string::const_iterator>(string.begin(), string.begin(), string.end());

                while (startIndex--)
                    head++;

                return std::string(head.base(), string.end());
            }

            std::string reduceLengthFromTail(size_t maxLength, const std::string& string)
            {
                auto tail = utf8::iterator<std::string::const_iterator>(string.end(), string.begin(), string.end());

                size_t length = utf8::distance(string.begin(), string.end());
                ASSERT(length > maxLength);

                size_t reduceLength = length - maxLength;

                while (reduceLength--)
                    tail--;

                return std::string(string.begin(), tail.base());
            }

            std::string sourceLocationNoteDiagnostic(const Diagnostic& diagnostic, size_t maxLineLength)
            {
                const auto sourceView = diagnostic.location.GetSourceView();
                ASSERT(sourceView);

                auto diagnosticLocation = sourceView->GetContentFrom(diagnostic.location);

                const bool diagnosicEOF = diagnosticLocation == sourceView->GetContent().end();
                if (diagnosicEOF)
                    --diagnosticLocation;

                UnownedStringSlice sourceLineSlice = sourceView->ExtractLineContainingLocation(diagnostic.location);

                // Trim any trailing white space
                sourceLineSlice = UnownedStringSlice(sourceLineSlice.begin(), trim(sourceLineSlice).end());

                // TODO(JS): The tab size should ideally be configurable from command line.
                // For now just go with 4.
                const uint32_t tabSize = 4;

                std::string lineToLocation = std::string(sourceLineSlice.begin(), diagnosticLocation);
                lineToLocation = replaceTabWithSpaces(lineToLocation, tabSize);
                size_t caretOffset = utf8::distance(lineToLocation.begin(), lineToLocation.end());
                caretOffset += diagnosicEOF ? 1 : 0;

                auto sourceLine = replaceTabWithSpaces(sourceLineSlice, tabSize);
                std::string caretLine;

                // Now the caretLine which appears underneath the sourceLine
                {
                    // Append spaces as many
                    caretLine.append(caretOffset, ' ');

                    // Add caret
                    caretLine += "^";

                    if (diagnostic.isTokenValid)
                    {
                        const auto length = diagnostic.stringSlice.length();

                        if (length > 1)
                            caretLine.append(length - 1, '~');
                    }

                    if (maxLineLength > 0)
                    {
                        const std::string ellipsis = "...";
                        const std::string spaces = "   ";
                        ASSERT(ellipsis.length() == spaces.length());
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
                                                                         std::string(sourceLine.begin() + ellipsisLen, sourceLine.end()));
                    }
                }

                // We could have handling here for if the line is too long, that we surround the important section
                // will ellipsis for example.
                // For now we just output.
                return fmt::format("{0}\n{1}\n", sourceLine, caretLine);
            }
        }

        std::string GetSeverityName(Severity severity)
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

        void DiagnosticSink::diagnoseImpl(const DiagnosticInfo& info, const std::string& formattedMessage)
        {
            if (info.severity >= Severity::Error)
                errorCount_++;

            for (const auto& writer : writerList_)
                writer->Write(formattedMessage);

            if (info.severity > Severity::Internal)
            {
                // TODO: figure out a better policy for aborting compilation
                PARSE_ABORT("");
            }
        }

        std::string DiagnosticSink::formatDiagnostic(const Diagnostic& diagnostic)
        {
            std::string humaneLocString;

            const bool includeSourceLocation = diagnostic.location.GetSourceView() != nullptr;

            if (includeSourceLocation)
            {
                HumaneSourceLocation humaneLocation = diagnostic.location.humaneSourceLoc;
                const auto sourceView = diagnostic.location.GetSourceView();
                ASSERT(sourceView);

                /*
                    CLion fomat
                    humaneLocString = fmt::format("{0}:{1}:{2}: ",
                                                  sourceView->GetSourceFile()->GetFileName(),
                                                  humaneLocation.line,
                                                  humaneLocation.column);
                */

                const auto path = onlyRelativePaths_ ? sourceView->GetPathInfo().foundPath : sourceView->GetPathInfo().uniqueIdentity;
                humaneLocString += fmt::format("{0}({1}): ",
                                               path,
                                               humaneLocation.line,
                                               humaneLocation.column);
            }

            std::string format = (diagnostic.errorID >= 0) ? "{0} {1}: {2}\n" : "{0}: {2}\n";

            std::string diagnosticString = humaneLocString + fmt::format(format,
                                                                      GetSeverityName(diagnostic.severity),
                                                                      diagnostic.errorID,
                                                                      diagnostic.message);

            if (includeSourceLocation)
            {
                auto sourceView = diagnostic.location.GetSourceView();

                diagnosticString += sourceLocationNoteDiagnostic(diagnostic, GetSourceLineMaxLength());

                while (sourceView->GetInitiatingSourceLocation().GetSourceView() &&
                       sourceView->GetPathInfo().type != PathInfo::Type::Normal &&
                       sourceView->GetPathInfo().type != PathInfo::Type::Split)
                {
                    diagnosticString += formatDiagnostic(sourceView->GetInitiatingToken(), Diagnostics::expandedFromMacro, sourceView->GetInitiatingToken().stringSlice);
                    sourceView = sourceView->GetInitiatingSourceLocation().GetSourceView();
                }
            }

            return diagnosticString;
        }
    }
}