#pragma once

#include <iostream>
#include <sstream>

namespace RR
{
    namespace Common
    {
        namespace Debug
        {
#if defined(OS_WINDOWS)
            template <class CharT, class TraitsT = std::char_traits<CharT>>
            class DebugStream : public std::basic_ostream<CharT, TraitsT>
            {
                using Base = std::basic_ostream<CharT, TraitsT>;

            public:
                DebugStream();
                ~DebugStream();
            };

            using ADebugStream = DebugStream<char>;
            using WDebugStream = DebugStream<wchar_t>;

            extern WDebugStream WStream;
            extern WDebugStream WErrorStream;
            extern ADebugStream Stream;
            extern ADebugStream ErrorStream;
#else
            static constexpr auto& WStream = std::wcout;
            static constexpr auto& WErrorStream = std::wcerr;
            static constexpr auto& Stream = std::cout;
            static constexpr auto& ErrorStream = std::cerr;
#endif
        }
    }
}