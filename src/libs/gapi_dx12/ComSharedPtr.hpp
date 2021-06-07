#pragma once

#include <winrt/base.h>

namespace RR
{
    template <typename T>
    using ComSharedPtr = winrt::com_ptr<T>;
}