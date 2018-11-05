#include <memory>

#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"

#include "Application.hpp"

void Application::Start() {
    Windowing::WindowSettings settings;
    Windowing::WindowRect rect(0, 0, 800, 600);

    settings.Title = "OpenDemo";
    settings.WindowRect = rect;

    Windowing::Windowing::Subscribe(this);
    Windowing::Windowing::CreateWindow(settings);

    while(!quit) {
        Windowing::Windowing::PoolEvents();
    }

    Windowing::Windowing::UnSubscribe(this);
}

void Application::Quit() {
    quit = true;
}
