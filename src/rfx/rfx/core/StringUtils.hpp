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
            auto end = const_cast<U8Char*>(slice.End());
            errno = 0;

            if (slice.StartsWith("0x"))
                outValue = static_cast<int64_t>(std::strtoull(slice.Begin(), &end, 16));
            else
                outValue = std::strtoll(slice.Begin(), &end, 0);

            if (errno == ERANGE)
                return RResult::ArithmeticOverflow;

            if (end != slice.End())
                return RResult::InvalidArgument;

            return RResult::Ok;
        }
    };
}