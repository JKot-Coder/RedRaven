#include "DebugStream.hpp"

#if defined(OS_WINDOWS)

#include <windows.h>

#include "backward.hpp"

namespace backward
{
    backward::SignalHandling sh;
} // namespace backward

namespace OpenDemo
{
    namespace Common
    {
        namespace Debug
        {
            WDebugStream WStream;
            WDebugStream WErrorStream;
            ADebugStream Stream;
            ADebugStream ErrorStream;

            template <class CharT, class TraitsT = std::char_traits<CharT>>
            class DebugStringBuffer : public std::basic_streambuf<CharT, TraitsT>
            {
            protected:
                inline std::streamsize xsputn(const CharT* s, std::streamsize n) override
                {
                    OutputDebugString(s);
                    return n;
                }

                inline int_type overflow(int_type c) override
                {
                    CharT str[2];
                    str[0] = c;
                    str[1] = 0;
                    OutputDebugString(&str[0]);
                    return c;
                }

                void OutputDebugString(const CharT* text) { }
            };

            template <>
            void DebugStringBuffer<char>::OutputDebugString(const char* text)
            {
                ::OutputDebugStringA(text);
            }

            template <>
            void DebugStringBuffer<wchar_t>::OutputDebugString(const wchar_t* text)
            {
                ::OutputDebugStringW(text);
            }

            template <class CharT, class TraitsT>
            DebugStream<CharT, TraitsT>::DebugStream()
                : std::basic_ostream<CharT, TraitsT>(new DebugStringBuffer<CharT, TraitsT>())
            {
            }

            template <class CharT, class TraitsT>
            DebugStream<CharT, TraitsT>::~DebugStream()
            {
                delete rdbuf();
            }
        }
    }
}
#endif