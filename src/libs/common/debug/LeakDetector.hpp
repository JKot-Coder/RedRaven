#pragma once

#include "common/Singleton.hpp"

namespace OpenDemo
{
    namespace Common
    {
        namespace Debug
        {
            struct MemorySnapshot;

            class LeakDetector : public Singleton<LeakDetector>
            {
            public:
                LeakDetector();
                ~LeakDetector();

                std::shared_ptr<MemorySnapshot> CreateEmpySnapshot() const;
                void Capture(const std::shared_ptr<MemorySnapshot>& oldSnapshot) const;
                uint64_t GetDifference(const std::shared_ptr<MemorySnapshot>& oldSnapshot, const std::shared_ptr<MemorySnapshot>& newSnapshot) const;

                void DumpAllSince(const std::shared_ptr<MemorySnapshot>& snapshot) const;
            };
        }
    }
}