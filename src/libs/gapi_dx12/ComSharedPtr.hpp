#pragma once

#include "common\ComPtr.hpp"

namespace RR
{
    template <typename T>
    using ComSharedPtr = Common::ComPtr<T>;
}