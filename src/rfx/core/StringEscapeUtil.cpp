#include "StringEscapeUtil.hpp"

#include "core/UnownedStringSlice.hpp"

#include <sstream>

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            char getHexChar(int v)
            {
                return (v <= 9) ? char(v + '0') : char(v - 10 + 'A');
            }

            char getCppEscapedChar(char c)
            {
                switch (c)
                {
                    // clang-format off
                    case '\b': return 'b';
                    case '\f': return 'f';
                    case '\n': return 'n';
                    case '\r': return 'r';
                    case '\a': return 'a';
                    case '\t': return 't';
                    case '\v': return 'v';
                    case '\'': return '\'';
                    case '\"': return '"';
                    case '\\': return '\\';
                    default:   return 0; // clang-format on
                }
            }

            char getJSONEscapedChar(char c)
            {
                switch (c)
                {
                    // clang-format off
                    case '\b': return 'b';
                    case '\f': return 'f';
                    case '\n': return 'n';
                    case '\r': return 'r';
                    case '\t': return 't';
                    case '\\': return '\\';
                    case '/':  return '/';
                    case '"':  return '"';
                    default: return 0; // clang-format on
                }
            }

            // Outputs ioSlice with the chars remaining after utf8 encoded value
            // Returns ~uint32_t(0) if can't decode
            uint32_t getUnicodePointFromUTF8(UnownedStringSlice& ioSlice)
            {
                const auto length = ioSlice.GetLength();
                ASSERT(length > 0);
                const auto cur = ioSlice.Begin();

                uint32_t codePoint = 0;
                unsigned int leading = cur[0];
                unsigned int mask = 0x80;

                int count = 0;
                while (leading & mask)
                {
                    count++;
                    mask >>= 1;
                }

                if (size_t(count) > length)
                {
                    ASSERT(!"Can't decode");
                    ioSlice = UnownedStringSlice(ioSlice.End(), ioSlice.End());
                    return ~uint32_t(0);
                }

                codePoint = (leading & (mask - 1));
                for (int i = 1; i <= count - 1; i++)
                {
                    codePoint <<= 6;
                    codePoint += (cur[i] & 0x3F);
                }

                ioSlice = UnownedStringSlice(cur + count, ioSlice.End());
                return codePoint;
            }

            void appendHex16(uint32_t value, U8String& out)
            {
                static const char s_hex[] = "0123456789abcdef";

                // Let's go with hex
                char buf[] = "\\u0000";

                buf[2] = s_hex[(value >> 12) & 0xf];
                buf[3] = s_hex[(value >> 8) & 0xf];
                buf[4] = s_hex[(value >> 4) & 0xf];
                buf[5] = s_hex[(value >> 0) & 0xf];

                out.append(buf, 6);
            }

            class StringEscapeHandler
            {
            public:
                StringEscapeHandler(char quoteChar) : quoteChar_(quoteChar)
                {
                }

                /// True if quoting is needed
                // virtual bool isQuotingNeeded(const UnownedStringSlice& slice) = 0;
                /// True if any escaping is needed. If not slice can be used (assuming appropriate quoting) as is
                // virtual bool isEscapingNeeded(const UnownedStringSlice& slice) = 0;
                /// True if we need to unescape
                // virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) = 0;

                /// Takes slice and adds any appropriate escaping (for example C++/C type escaping for special characters like '\', '"' and if not ascii will write out as hex sequence)
                /// Does not append quotes
                virtual void AppendEscaped(const UnownedStringSlice& slice, U8String& out) const = 0;
                /// Given a slice append it unescaped
                /// Does not consume surrounding quotes
                //virtual void AppendUnescaped(const UnownedStringSlice& slice, U8String& out) const = 0;

                /// Lex quoted text.
                /// The first character of cursor should be the quoteCharacter.
                /// cursor points to the string to be lexed - must typically be 0 terminated.
                /// outCursor on successful lex will be at the next character after was processed.
                //virtual SlangResult lexQuoted(const char* cursor, const char** outCursor) = 0;

                inline char GetQuoteChar() const { return quoteChar_; }

            protected:
                const char quoteChar_;
            };

            // !!!!!!!!!!!!!!!!!!!!!!!!!! CppStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            class CppStringEscapeHandler final : public StringEscapeHandler
            {
            public:
                typedef StringEscapeHandler Super;

                /* virtual bool isQuotingNeeded(const UnownedStringSlice& slice) override
                {
                    SLANG_UNUSED(slice);
                    return true;
                }*/

                //  virtual bool isEscapingNeeded(const UnownedStringSlice& slice) override;
                //   virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) override;
                virtual void AppendEscaped(const UnownedStringSlice& slice, U8String& out) const override;
                // virtual void AppendUnescaped(const UnownedStringSlice& slice, U8String& out) const override;
                //  virtual RfxResult lexQuoted(const char* cursor, const char** outCursor) override;

                CppStringEscapeHandler() : Super('"') { }
            };

            void CppStringEscapeHandler::AppendEscaped(const UnownedStringSlice& slice, U8String& out) const
            {
                // TODO: UTF-8 support here
                const char* start = slice.Begin();
                const char* cur = start;
                const char* const end = slice.End();

                for (; cur < end; ++cur)
                {
                    const char c = *cur;
                    const char escapedChar = getCppEscapedChar(c);

                    if (escapedChar)
                    {
                        // Flush
                        if (start < cur)
                            out.append(start, cur);

                        out.push_back('\\');
                        out.push_back(escapedChar);
                        start = cur + 1;
                    }
                    else if (c < ' ' || c > 126)
                    {
                        // Flush
                        if (start < cur)
                        {
                            out.append(start, cur);
                        }

                        char buf[5] = "\\0x0";
                        buf[3] = getHexChar((int(c) >> 4) & 0xf);
                        buf[4] = getHexChar(c & 0xf);

                        out.append(buf, buf + 4);

                        start = cur + 1;
                    }
                }

                if (start < end)
                    out.append(start, end);
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!! JSONStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            class JSONStringEscapeHandler final : public StringEscapeHandler
            {
            public:
                typedef StringEscapeHandler Super;

                /* virtual bool isQuotingNeeded(const UnownedStringSlice& slice) override
                {
                    SLANG_UNUSED(slice);
                    return true;
                }*/
                //  virtual bool isEscapingNeeded(const UnownedStringSlice& slice) override;
                // virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) override;
                virtual void AppendEscaped(const UnownedStringSlice& slice, U8String& out) const override;
                //virtual RfxResult appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out) override;
                //virtual RfxResult lexQuoted(const char* cursor, const char** outCursor) override;

                JSONStringEscapeHandler() : Super('"') { }
            };

            void JSONStringEscapeHandler::AppendEscaped(const UnownedStringSlice& slice, U8String& out) const
            {
                auto start = slice.Begin();
                auto cur = start;
                const auto end = slice.End();

                for (; cur < end; ++cur)
                {
                    const auto c = *cur;
                    const auto escapedChar = getJSONEscapedChar(c);

                    if (escapedChar)
                    {
                        // Flush
                        if (start < cur)
                            out.append(start, cur);

                        out.push_back('\\');
                        out.push_back(escapedChar);

                        start = cur + 1;
                    }
                    else if (uint8_t(c) & 0x80)
                    {
                        // Flush
                        if (start < cur)
                            out.append(start, cur);

                        // UTF8
                        UnownedStringSlice remainingSlice(cur, end);
                        uint32_t codePoint = getUnicodePointFromUTF8(remainingSlice);

                        // We only support up to 16 bit unicode values for now...
                        ASSERT(codePoint < 0x10000);

                        appendHex16(codePoint, out);

                        cur = remainingSlice.Begin() - 1;
                        start = cur + 1;
                    }
                    else if (uint8_t(c) < ' ' || (c >= 0x7e))
                    {
                        if (start < cur)
                        {
                            out.append(start, cur);
                        }

                        appendHex16(uint32_t(c), out);

                        start = cur + 1;
                    }
                    else
                    {
                        // Can go out as it is
                    }
                }

                // Flush at the end
                if (start < end)
                    out.append(start, end);
            }

            StringEscapeHandler* getHandler(StringEscapeUtil::Style style)
            {
                static CppStringEscapeHandler cppHandler;
                static JSONStringEscapeHandler jsonHandler;

                switch (style)
                {
                    case StringEscapeUtil::Style::Cpp:
                        return &cppHandler;

                    case StringEscapeUtil::Style::JSON:
                        return &jsonHandler;

                    default:
                        ASSERT_MSG(false, "Unexpected string escape style");
                        return nullptr;
                }
            }
        }

        /// Takes slice and adds any appropriate escaping (for example C++/C type escaping for special characters like '\', '"' and if not ascii will write out as hex sequence)
        /// Does not append quotes
        void StringEscapeUtil::AppendEscaped(Style style, const UnownedStringSlice& slice, U8String& out)
        {
            const auto handler = getHandler(style);
            handler->AppendEscaped(slice, out);
        }

        /* 
        /// Given a slice append it unescaped
        /// Does not consume surrounding quotes
        RfxResult StringEscapeUtil::AppendUnescaped(Style style, const UnownedStringSlice& slice, std::stringstream& out);

        /// If quoting is needed appends to out quoted
        RfxResult StringEscapeUtil::AppendMaybeQuoted(Style style, const UnownedStringSlice& slice, std::stringstream& out);

        /// If the slice appears to be quoted for the style, unquote it, else just append to out
        RfxResult StringEscapeUtil::AppendMaybeUnquoted(Style style, const UnownedStringSlice& slice, std::stringstream& out);

        /// Appends to out slice without quotes
        RfxResult StringEscapeUtil::AppendUnquoted(Style style, const UnownedStringSlice& slice, std::stringstream& out);
        */
        /// Append with quotes (even if not needed)
        void StringEscapeUtil::AppendQuoted(Style style, const UnownedStringSlice& slice, U8String& out)
        {
            const auto handler = getHandler(style);
            out.push_back(handler->GetQuoteChar());
            handler->AppendEscaped(slice, out);
            out.push_back(handler->GetQuoteChar());
        }

        /*
        /// True is slice is quoted
        bool StringEscapeUtil::IsQuoted(Style style, UnownedStringSlice& slice);

        /// True if requires 'shell-like' unescape. With shell-like, quoting does *not* have to start at the start of the slice.
        /// and there may be multiple quoted section
        RfxResult StringEscapeUtil::IsUnescapeShellLikeNeeded(Style style, const UnownedStringSlice& slice);

        /// Shells can have multiple quoted sections. This function makes a string with out quoting
        RfxResult StringEscapeUtil::UnescapeShellLike(Style style, const UnownedStringSlice& slice, std::stringstream& out);*/

    }
}