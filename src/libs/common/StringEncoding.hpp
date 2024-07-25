#pragma once

#include "utf8.h"
#include <string>

namespace RR
{
    namespace StringEncoding
    {
        template <typename wchar_iterator, typename octet_iterator>
        octet_iterator WideToUTF8(wchar_iterator start, wchar_iterator end, octet_iterator result)
        {
            static_assert(sizeof(typename std::iterator_traits<wchar_iterator>::value_type) == sizeof(wchar_t));
            if constexpr (sizeof(wchar_t) == 16)
                return utf8::utf16to8(start, end, result);

            return utf8::utf32to8(start, end, result);
        }

        inline std::string WideToUTF8(std::wstring_view view)
        {
            std::string result;

            if (view.empty())
                return result;

            WideToUTF8(view.begin(), view.end(), std::back_inserter(result));
            return result;
        }

         inline std::string WideToUTF8(const std::wstring& string)
        {
            std::string result;

            if (string.empty())
                return result;

            WideToUTF8(string.begin(), string.end(), std::back_inserter(result));
            return result;
        }

        template <typename octet_iterator, typename wchar_iterator>
        wchar_iterator UTF8ToWide(octet_iterator start, octet_iterator end, wchar_iterator result)
        {
            if constexpr (sizeof(wchar_t) == 16)
                return utf8::utf8to16(start, end, result);

            return utf8::utf8to32(start, end, result);
        }

        inline std::wstring UTF8ToWide(const std::string_view& view)
        {
            std::wstring result;

            if (view.empty())
                return result;

            UTF8ToWide(view.begin(), view.end(), std::back_inserter(result));
            return result;
        }

        inline std::wstring UTF8ToWide(const std::string& string)
        {
            std::wstring result;

            if (string.empty())
                return result;

            UTF8ToWide(string.begin(), string.end(), std::back_inserter(result));
            return result;
        }
    }
}