#pragma once

#include "common/Result.hpp"
#include "rfx/core/UnownedStringSlice.hpp"
#include <cstdlib>

namespace RR::Rfx
{
    using RResult = Common::RResult;

    struct StringUtils
    {
        static RResult StringToInt64(const UnownedStringSlice& slice, int64_t& outValue)
        {
            auto end = const_cast<U8Char*>(slice.end());
            errno = 0;

            if (slice.isStartsWith("0x"))
                outValue = static_cast<int64_t>(std::strtoull(slice.begin(), &end, 16));
            else
                outValue = std::strtoll(slice.begin(), &end, 0);

            if (errno == ERANGE)
                return RResult::ArithmeticOverflow;

            if (end != slice.end())
                return RResult::InvalidArgument;

            return RResult::Ok;
        }
    };
}