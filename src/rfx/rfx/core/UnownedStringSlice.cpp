#include "UnownedStringSlice.hpp"

namespace RR
{
    namespace Rfx
    {
        bool UnownedStringSlice::operator==(UnownedStringSlice const& other) const
        {
            // Note that memcmp is undefined when passed in null ptrs, so if we want to handle
            // we need to cover that case.
            // Can only be nullptr if size is 0.
            auto thisSize = GetLength();
            auto otherSize = other.GetLength();

            if (thisSize != otherSize)
            {
                return false;
            }

            const U8Char* const thisChars = Begin();
            const U8Char* const otherChars = other.Begin();
            if (thisChars == otherChars || thisSize == 0)
            {
                return true;
            }
            ASSERT(thisChars && otherChars);
            return memcmp(thisChars, otherChars, thisSize) == 0;
        }
    }
}