#include "Application.hpp"
#include "windowing/Windowing.hpp"

#ifdef OS_WINDOWS
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

#else
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
#endif

    auto app = new OpenDemo::Application;

    app->Start();

    return 0;
}