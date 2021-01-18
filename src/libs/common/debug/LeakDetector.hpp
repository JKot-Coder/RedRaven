#pragma once

#include "common/Singleton.hpp"

namespace OpenDemo
{
    namespace Common
    {
        namespace Debug
        {
            class LeakDetector : public Singleton<LeakDetector>
            {
            public:
                struct Snapshot;

            public:
                LeakDetector();
                ~LeakDetector();

                std::shared_ptr<LeakDetector::Snapshot> CreateEmpySnapshot() const;
                void Capture(const std::shared_ptr<LeakDetector::Snapshot>& oldSnapshot) const;
                uint64_t GetDifference(const std::shared_ptr<LeakDetector::Snapshot>& oldSnapshot, const std::shared_ptr<LeakDetector::Snapshot>& newSnapshot) const;

                void DumpAllSince(const std::shared_ptr<Snapshot>& snapshot) const;
            };
        }
    }
}