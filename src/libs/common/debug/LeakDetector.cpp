#include "LeakDetector.hpp"

#if OS_WINDOWS

#endif

namespace OpenDemo
{
    namespace Common
    {
        namespace Debug
        {
            struct LeakDetector::Snapshot
            {
#if OS_WINDOWS
                _CrtMemState state;
#endif
                bool inited_ = false;
            };

            LeakDetector::LeakDetector()
            {
#if OS_WINDOWS
                int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
                flag |= _CRTDBG_LEAK_CHECK_DF;
                flag |= _CRTDBG_ALLOC_MEM_DF;
                _CrtSetDbgFlag(flag);
                _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
                _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
                //_CrtSetBreakAlloc(-1);
#endif
            }

            LeakDetector::~LeakDetector()
            {
            }

            std::shared_ptr<LeakDetector::Snapshot> LeakDetector::CreateEmpySnapshot() const
            {
                return std::make_shared<LeakDetector::Snapshot>();
            }

            void LeakDetector::Capture(const std::shared_ptr<LeakDetector::Snapshot>& snapshot) const
            {
                ASSERT(snapshot);
                ASSERT(!snapshot->inited_);

#if OS_WINDOWS
                _CrtMemCheckpoint(&snapshot->state);
#endif

                snapshot->inited_ = true;
            }

            uint64_t LeakDetector::GetDifference(const std::shared_ptr<LeakDetector::Snapshot>& oldSnapshot, const std::shared_ptr<LeakDetector::Snapshot>& newSnapshot) const
            {
                ASSERT(oldSnapshot);
                ASSERT(newSnapshot);
                ASSERT(oldSnapshot->inited_);
                ASSERT(newSnapshot->inited_);

#if OS_WINDOWS
                _CrtMemState diffMemState;

                _CrtMemDifference(
                    &diffMemState,
                    &oldSnapshot->state,
                    &newSnapshot->state);

                return diffMemState.lTotalCount;
#endif
            }

            void LeakDetector::DumpAllSince(const std::shared_ptr<LeakDetector::Snapshot>& snapshot) const
            {
                ASSERT(snapshot);
                ASSERT(snapshot->inited_);
#if OS_WINDOWS
                _CrtMemDumpAllObjectsSince(&snapshot->state);
#endif
            }

        }
    }
}