#include <memory>
#include <render/Rendering.hpp>

#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"

#include "Application.hpp"

void Application::Start() {
    init();

    while(!quit) {
        Windowing::Windowing::PoolEvents();
    }

    terminate();
}

void Application::Quit() {
    quit = true;
}

void Application::init() {
    Windowing::WindowSettings settings;
    Windowing::WindowRect rect(0, 0, 800, 600);

    settings.Title = "OpenDemo";
    settings.WindowRect = rect;

    Windowing::Windowing::Subscribe(this);
    window = Windowing::Windowing::CreateWindow(settings);

    Render::Instance().get()->Init(window);
}

void Application::terminate() {
    window.reset();
    window = nullptr;

    Render::Instance().get()->Terminate();
    Windowing::Windowing::UnSubscribe(this);
}

