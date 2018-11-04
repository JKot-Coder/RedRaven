#include "windowing/Windowing.hpp"
#include "windowing/WindowSettings.hpp"

int main(int argc, char** argv) {
    Windowing::WindowSettings settings;
    Windowing::WindowRect rect(0, 0, 800, 600);

    settings.Title = "OpenDemo";
    settings.WindowRect = rect;

    Windowing::Windowing::CreateWindow(settings);

    while(true) {
        Windowing::Windowing::PoolEvents();
    }
}

