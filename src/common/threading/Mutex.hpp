#pragma once

#include <mutex>

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            // Type alias
            using Mutex = std::mutex;
            // Type alias
            using RecursiveMutex = std::recursive_mutex;
        }
    }
}