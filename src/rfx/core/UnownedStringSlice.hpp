#pragma once

namespace RR
{
    namespace Rfx
    {
        struct UnownedStringSlice
        {
        public:
            UnownedStringSlice() = default;

            UnownedStringSlice(const U8String& string)
                : begin_(string.c_str()), end_(string.c_str() + string.length())
            {
            }

            UnownedStringSlice(const U8Char* begin, const U8Char* end)
                : begin_(begin), end_(end)
            {
            }

            const U8Char* Begin() const { return begin_; }
            const U8Char* End() const { return end_; }

            /*
                /// True if slice is strictly contained in memory.
                bool IsMemoryContained(const UnownedStringSlice& slice) const
                {

                    return slice.begin_ >= begin_ && slice.end_ <= end_;
                }

                bool isMemoryContained(const char* pos) const
                {
                    return pos >= m_begin && pos <= m_end;
                }
                */

            size_t GetLength() const
            {
                return std::distance(begin_, end_);
            }

            /*
                /// Finds first index of char 'c'. If not found returns -1.
                Index indexOf(char c) const;
                /// Find first index of slice. If not found returns -1
                Index indexOf(const UnownedStringSlice& slice) const;

                /// Returns a substring. idx is the start index, and len
                /// is the amount of characters.
                /// The returned length might be truncated, if len extends beyond slice.
                UnownedStringSlice subString(Index idx, Index len) const;

                /// Return a head of the slice - everything up to the index
                SLANG_FORCE_INLINE UnownedStringSlice head(Index idx) const
                {
                    SLANG_ASSERT(idx >= 0 && idx <= getLength());
                    return UnownedStringSlice(m_begin, idx);
                }
                /// Return a tail of the slice - everything from the index to the end of the slice
                SLANG_FORCE_INLINE UnownedStringSlice tail(Index idx) const
                {
                    SLANG_ASSERT(idx >= 0 && idx <= getLength());
                    return UnownedStringSlice(m_begin + idx, m_end);
                }

                /// True if rhs and this are equal without having to take into account case
                /// Note 'case' here is *not* locale specific - it is only A-Z and a-z
                bool caseInsensitiveEquals(const ThisType& rhs) const;

                Index lastIndexOf(char c) const
                {
                    const Index size = Index(m_end - m_begin);
                    for (Index i = size - 1; i >= 0; --i)
                    {
                        if (m_begin[i] == c)
                        {
                            return i;
                        }
                    }
                    return -1;
                }

                const char& operator[](Index i) const
                {
                    assert(i >= 0 && i < Index(m_end - m_begin));
                    return m_begin[i];
                }*/

            bool operator==(UnownedStringSlice const& other) const;
            bool operator!=(UnownedStringSlice const& other) const
            {
                return !(*this == other);
            }

            bool operator==(char const* str) const
            {
                return (*this) == UnownedStringSlice(str);
            }
            bool operator!=(char const* str) const
            {
                return !(*this == str);
            } /*

                /// True if contents is a single char of c
                SLANG_FORCE_INLINE bool isChar(char c) const
                {
                    return getLength() == 1 && m_begin[0] == c;
                }

                bool startsWith(UnownedStringSlice const& other) const;
                bool startsWith(char const* str) const;

                bool endsWith(UnownedStringSlice const& other) const;
                bool endsWith(char const* str) const;
*/
            /// Trims any horizontal whitespace from the start and end and returns as a substring
            UnownedStringSlice Trim() const;
            /*    /// Trims any 'c' from the start or the end, and returns as a substring
                UnownedStringSlice trim(char c) const;

                /// Trims any horizonatl whitespace from start and returns as a substring
                UnownedStringSlice trimStart() const;

                HashCode getHashCode() const
                {
                    return Slang::getHashCode(m_begin, size_t(m_end - m_begin));
                }

                template <size_t SIZE>
                SLANG_FORCE_INLINE static UnownedStringSlice fromLiteral(const char (&in)[SIZE])
                {
                    return UnownedStringSlice(in, SIZE - 1);
                } 
                */

        private:
            const U8Char* begin_;
            const U8Char* end_;
        };
    }
}