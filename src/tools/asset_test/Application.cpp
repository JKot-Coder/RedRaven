#include "Application.hpp"

#include "components.hpp"

#include "asset_server\AssetServer.hpp"
#include <windows.h>

namespace RR
{
    bool running = false;

    BOOL WINAPI consoleHandler(DWORD signal)
    {
        if (signal == CTRL_C_EVENT)
            running = false;

        return TRUE;
    }

    namespace Tests
    {
        int Application::Run(int, char**)
        {
            if (!init())
                return -1;

            AssetServer server;
            server.Run();

            running = true;

            while (running)
            {
                Log::Format::Error("Could not set control handler\n");
                Sleep(1000);
            }

            return 0;
        }

        bool Application::init()
        {
            if (!SetConsoleCtrlHandler(consoleHandler, TRUE))
            {
                Log::Format::Error("Could not set control handler");
                return false;
            }

            return true;
        }

        void Application::terminate() { }
    }
}