#pragma once

#pragma warning(push)
#pragma warning(disable : 26451)
#pragma warning(disable : 26812)
#include "dependencies/fmt/include/fmt/format.h"
#include "dependencies/fmt/include/fmt/printf.h"
#pragma warning(pop)

#include "dependencies/utfcpp/source/utf8.h"
#include <string>

namespace RR
{
#if __cplusplus > 201703L
    // C++20
    using U8String = std::u8string;
    using U8Char = char8_t;
    using U8Glyph = uint32_t;
#else
    using U8String = std::string;
    using U8Char = char;
    using U8Glyph = uint32_t;
#endif

    namespace StringConversions
    {
        inline U8String WStringToUTF8(const std::wstring& string)
        {
            U8String result;

            if (string.empty())
                return result;

#ifdef _MSC_VER
            utf8::utf16to8(string.begin(), string.end(), std::back_inserter(result));
#else
            utf8::utf32to8(string.begin(), string.end(), std::back_inserter(result));
#endif
            return result;
        }

        inline std::wstring UTF8ToWString(const U8String& string)
        {
            std::wstring result;

            if (string.empty())
                return result;

#ifdef _MSC_VER
            utf8::utf8to16(string.begin(), string.end(), std::back_inserter(result));
#else
            utf8::utf8to16(string.begin(), string.end(), std::back_inserter(result));
#endif
            return result;
        }
    }
}