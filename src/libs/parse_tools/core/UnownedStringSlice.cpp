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
            auto thisSize = length();
            auto otherSize = other.length();

            if (thisSize != otherSize)
                return false;

            const char* const thisChars = begin();
            const char* const otherChars = other.begin();
            if (thisChars == otherChars || thisSize == 0)
                return true;

            ASSERT(thisChars && otherChars);
            return memcmp(thisChars, otherChars, thisSize) == 0;
        }
    }
}