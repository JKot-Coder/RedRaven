#pragma once

#include <string>
#include "dependencies/fmt/include/fmt/format.h"
#include "dependencies/utfcpp/source/utf8.h"

namespace OpenDemo
{
    #if __cplusplus > 201703L
    // C++20
    using U8String = std::u8string;
    #else
    using U8String = std::string;
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