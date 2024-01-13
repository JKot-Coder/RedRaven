#include "StringEscapeUtil.hpp"

#include "rfx/core/UnownedStringSlice.hpp"

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

            char getCppEscapedChar(U8Glyph c)
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

            char getJSONEscapedChar(U8Glyph c)
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
            /*   uint32_t getUnicodePointFromUTF8(UnownedStringSlice& ioSlice)
               {
                   const auto length = ioSlice.length();
                   ASSERT(length > 0);
                   const auto cur = ioSlice.begin();

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
                       ioSlice = UnownedStringSlice(ioSlice.end(), ioSlice.end());
                       return ~uint32_t(0);
                   }

                   codePoint = (leading & (mask - 1));
                   for (int i = 1; i <= count - 1; i++)
                   {
                       codePoint <<= 6;
                       codePoint += (cur[i] & 0x3F);
                   }

                   ioSlice = UnownedStringSlice(cur + count, ioSlice.end());
                   return codePoint;
               }*/
            void appendHex16(uint32_t value, U8String& out)
            {
                // Let's go with hex
                char buf[] = "\\u0000";

                buf[2] = getHexChar((value >> 12) & 0xf);
                buf[3] = getHexChar((value >> 8) & 0xf);
                buf[4] = getHexChar((value >> 4) & 0xf);
                buf[5] = getHexChar((value >> 0) & 0xf);

                out.append(buf, 6);
            }

            void appendHex8(uint32_t value, U8String& out)
            {
                // Let's go with hex
                char buf[5] = "\\0x0";
                buf[3] = getHexChar((value >> 4) & 0xf);
                buf[4] = getHexChar((value >> 0) & 0xf);

                out.append(buf, buf + 4);
            }

            class StringEscapeHandler
            {
            public:
                StringEscapeHandler(char quoteChar) : quoteChar_(quoteChar)
                {
                }

                /// True if quoting is needed
                // virtual bool isQuotingNeeded(UnownedStringSlice slice) = 0;
                /// True if any escaping is needed. If not slice can be used (assuming appropriate quoting) as is
                // virtual bool isEscapingNeeded(UnownedStringSlice slice) = 0;
                /// True if we need to unescape
                // virtual bool isUnescapingNeeeded(UnownedStringSlice slice) = 0;

                /// Takes slice and adds any appropriate escaping (for example C++/C type escaping for special characters like '\', '"' and if not ascii will write out as hex sequence)
                /// Does not append quotes
                virtual void AppendEscaped(UnownedStringSlice slice, U8String& out) const = 0;
                /// Given a slice append it unescaped
                /// Does not consume surrounding quotes
                // virtual void AppendUnescaped(UnownedStringSlice slice, U8String& out) const = 0;

                /// Lex quoted text.
                /// The first character of cursor should be the quoteCharacter.
                /// cursor points to the string to be lexed - must typically be 0 terminated.
                /// outCursor on successful lex will be at the next character after was processed.
                // virtual SlangResult lexQuoted(const char* cursor, const char** outCursor) = 0;

                inline char GetQuoteChar() const { return quoteChar_; }

            protected:
                const char quoteChar_;
            };

            // !!!!!!!!!!!!!!!!!!!!!!!!!! CppStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            class CppStringEscapeHandler final : public StringEscapeHandler
            {
            public:
                typedef StringEscapeHandler Super;

                /* virtual bool isQuotingNeeded(UnownedStringSlice slice) override
                {
                    SLANG_UNUSED(slice);
                    return true;
                }*/

                //  virtual bool isEscapingNeeded(UnownedStringSlice slice) override;
                //   virtual bool isUnescapingNeeeded(UnownedStringSlice slice) override;
                virtual void AppendEscaped(UnownedStringSlice slice, U8String& out) const override;
                // virtual void AppendUnescaped(UnownedStringSlice slice, U8String& out) const override;
                //  virtual RfxResult lexQuoted(const char* cursor, const char** outCursor) override;

                CppStringEscapeHandler() : Super('"') { }
            };

            void CppStringEscapeHandler::AppendEscaped(UnownedStringSlice slice, U8String& out) const
            {
                const auto start = slice.begin();
                auto cur = start;
                const auto end = slice.end();

                for (; cur < end;)
                {
                    const auto ch = utf8::next(cur, end);
                    const auto escapedChar = getCppEscapedChar(ch);

                    if (escapedChar)
                    {
                        out.push_back('\\');
                        out.push_back(escapedChar);
                    }
                    else if (ch < ' ')
                    {
                        appendHex8(uint32_t(ch), out);
                    }
                    else if (ch > 126)
                    {
                        appendHex16(uint32_t(ch), out);
                    }
                    else
                        utf8::append(static_cast<U8Glyph>(ch), out);
                }
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!! JSONStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            class JSONStringEscapeHandler final : public StringEscapeHandler
            {
            public:
                typedef StringEscapeHandler Super;

                /* virtual bool isQuotingNeeded(UnownedStringSlice slice) override
                {
                    SLANG_UNUSED(slice);
                    return true;
                }*/
                //  virtual bool isEscapingNeeded(UnownedStringSlice slice) override;
                // virtual bool isUnescapingNeeeded(UnownedStringSlice slice) override;
                virtual void AppendEscaped(UnownedStringSlice slice, U8String& out) const override;
                // virtual RfxResult appendUnescaped(UnownedStringSlice slice, StringBuilder& out) override;
                // virtual RfxResult lexQuoted(const char* cursor, const char** outCursor) override;

                JSONStringEscapeHandler() : Super('"') { }
            };

            void JSONStringEscapeHandler::AppendEscaped(UnownedStringSlice slice, U8String& out) const
            {
                const auto start = slice.begin();
                auto cur = start;
                const auto end = slice.end();

                for (; cur < end;)
                {
                    const auto ch = utf8::next(cur, end);
                    const auto escapedChar = getJSONEscapedChar(ch);

                    if (escapedChar)
                    {
                        out.push_back('\\');
                        out.push_back(escapedChar);
                    }
                    else if (ch < ' ')
                    {
                        appendHex16(uint32_t(ch), out);
                    }
                    else
                        utf8::append(static_cast<U8Glyph>(ch), out);
                }
            }

            StringEscapeHandler* getHandler(StringEscapeUtil::Style style)
            {
                static CppStringEscapeHandler cppHandler;
                static JSONStringEscapeHandler jsonHandler;

                switch (style)
                {
                    case StringEscapeUtil::Style::Cpp: return &cppHandler;
                    case StringEscapeUtil::Style::JSON: return &jsonHandler;
                    default:
                        ASSERT_MSG(false, "Unexpected string escape style");
                        return nullptr;
                }
            }
        }

        /// Takes slice and adds any appropriate escaping (for example C++/C type escaping for special characters like '\', '"' and if not ascii will write out as hex sequence)
        /// Does not append quotes
        void StringEscapeUtil::AppendEscaped(Style style, UnownedStringSlice slice, U8String& out)
        {
            const auto handler = getHandler(style);
            handler->AppendEscaped(slice, out);
        }

        /*
        /// Given a slice append it unescaped
        /// Does not consume surrounding quotes
        RfxResult StringEscapeUtil::AppendUnescaped(Style style, UnownedStringSlice slice, std::stringstream& out);

        /// If quoting is needed appends to out quoted
        RfxResult StringEscapeUtil::AppendMaybeQuoted(Style style, UnownedStringSlice slice, std::stringstream& out);

        /// If the slice appears to be quoted for the style, unquote it, else just append to out
        RfxResult StringEscapeUtil::AppendMaybeUnquoted(Style style, UnownedStringSlice slice, std::stringstream& out);

        /// Appends to out slice without quotes
        RfxResult StringEscapeUtil::AppendUnquoted(Style style, UnownedStringSlice slice, std::stringstream& out);
        */
        /// Append with quotes (even if not needed)
        void StringEscapeUtil::AppendQuoted(Style style, UnownedStringSlice slice, U8String& out)
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
        RfxResult StringEscapeUtil::IsUnescapeShellLikeNeeded(Style style, UnownedStringSlice slice);

        /// Shells can have multiple quoted sections. This function makes a string with out quoting
        RfxResult StringEscapeUtil::UnescapeShellLike(Style style, UnownedStringSlice slice, std::stringstream& out);*/
    }
}