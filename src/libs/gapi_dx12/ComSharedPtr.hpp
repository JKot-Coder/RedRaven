#pragma once

#include <winrt/base.h>

namespace OpenDemo
{
    template <typename T>
    using ComSharedPtr = winrt::com_ptr<T>;
}