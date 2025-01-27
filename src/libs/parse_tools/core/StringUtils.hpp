#pragma once

#include "common/Result.hpp"
#include "rfx/core/UnownedStringSlice.hpp"
#include <cstdlib>

namespace RR::Rfx
{
    using RResult = Common::RResult;

    struct StringUtils
    {
        // Todo better implemenation
        static RResult StringToInt64(UnownedStringSlice string, int64_t& outValue) { return StringToInt64(string.asString(), outValue); }
        static RResult StringToInt64(std::string string, int64_t& outValue)
        {
            char* end;
            errno = 0;

            if (string.rfind("0x", 0) == 0)
                outValue = static_cast<int64_t>(std::strtoull(string.c_str(), &end, 16));
            else
                outValue = std::strtoll(string.c_str(), &end, 0);

            if (errno == ERANGE)
                return RResult::ArithmeticOverflow;

            if (end != string.data() + string.size())
                return RResult::InvalidArgument;

            return RResult::Ok;
        }
    };
}