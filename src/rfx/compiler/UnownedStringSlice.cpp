#include "UnownedStringSlice.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            UnownedStringSlice UnownedStringSlice::Trim() const
            {
                const U8Char* start = begin_;
                const U8Char* end = end_;

                // Work with UTF8 as ANSI text. This shouldn't be a problem...
                while (start < end && (*start == '\t' || *start == ' ')) start++;
                while (end > start && (*start == '\t' || *start == ' ')) end--;

                return UnownedStringSlice(start, end);
            }
        }
    }
}
