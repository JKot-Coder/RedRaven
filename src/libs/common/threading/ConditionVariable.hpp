#pragma once

#include <condition_variable>

namespace RR
{
    namespace Common
    {
        namespace Threading
        {
            // Type alias
            using ConditionVariable = std::condition_variable;
        }
    }
}