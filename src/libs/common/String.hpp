#pragma once

#include "utf8.h"
#include <string>

namespace RR
{
    namespace StringConversions
    {
        inline std::string WStringToUTF8(const std::wstring& string)
        {
            std::string result;

            if (string.empty())
                return result;

#ifdef _MSC_VER
            utf8::utf16to8(string.begin(), string.end(), std::back_inserter(result));
#else
            utf8::utf32to8(string.begin(), string.end(), std::back_inserter(result));
#endif
            return result;
        }

        inline std::wstring UTF8ToWString(const std::string& string)
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