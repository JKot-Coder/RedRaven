#pragma once

namespace RR
{
    namespace ParseTools
    {
        struct UnownedStringSlice;

        /* A set of function that can be used for escaping/unescaping quoting/unquoting strings.

        The distinction between 'escaping' and 'quoting' here, is just that escaping is the 'payload' of quotes.
        In *principal* the Style can determine different styles of escaping that can be used.
        */
        struct StringEscapeUtil
        {
            enum class Style
            {
                Cpp, ///< Cpp style quoting and escape handling
                Space, ///< Applies quotes if there are spaces. Does not escape.
                JSON, ///< Json encoding
            };

            /// Takes slice and adds any appropriate escaping (for example C++/C type escaping for special characters like '\', '"' and if not ascii will write out as hex sequence)
            /// Does not append quotes
            static void AppendEscaped(Style style, UnownedStringSlice slice, std::string& out);
            /*
            /// Given a slice append it unescaped
            /// Does not consume surrounding quotes
            static RfxResult AppendUnescaped(Style style, UnownedStringSlice slice, std::stringstream& out);

            /// If quoting is needed appends to out quoted
            static RfxResult AppendMaybeQuoted(Style style, UnownedStringSlice slice, std::stringstream& out);

            /// If the slice appears to be quoted for the style, unquote it, else just append to out
            static RfxResult AppendMaybeUnquoted(Style style, UnownedStringSlice slice, std::stringstream& out);

            /// Appends to out slice without quotes
            static RfxResult AppendUnquoted(Style style, UnownedStringSlice slice, std::stringstream& out);
            */
            /// Append with quotes (even if not needed)
            static void AppendQuoted(Style style, UnownedStringSlice slice, std::string& out);
            /*
            /// True is slice is quoted
            static bool IsQuoted(Style style, UnownedStringSlice& slice);

            /// True if requires 'shell-like' unescape. With shell-like, quoting does *not* have to start at the start of the slice.
            /// and there may be multiple quoted section
            static RfxResult IsUnescapeShellLikeNeeded(Style style, UnownedStringSlice slice);

            /// Shells can have multiple quoted sections. This function makes a string with out quoting
            static RfxResult UnescapeShellLike(Style style, UnownedStringSlice slice, std::stringstream& out);

            /// Get without quotes. Will assert if not correctly quoted
            static UnownedStringSlice Unquote(Style style, UnownedStringSlice slice);  */
        };
    }
}