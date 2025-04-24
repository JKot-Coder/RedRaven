#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>
#if OS_WINDOWS
#include <Windows.h>
#endif

namespace
{
    int runApp(int argc, char** argv)
    {
#if OS_WINDOWS
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        DWORD_PTR mask = 0;
        // Set the mask to only include P-cores (assuming the first few cores are P-cores)
        // You'll need to adjust this according to your CPU's P-core/E-core mapping.
        for (DWORD i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
        {
            if (i < 8)
            { // Assuming the first 8 cores are P-cores
                mask |= (1 << i);
            }
        }

        // Set the processor affinity
        SetThreadAffinityMask(GetCurrentThread(), mask);
#endif
        auto session = Catch::Session();
        return session.run(argc, argv);
    }
}

int main(int argc, char** argv)
{
    return runApp(argc, argv);
}