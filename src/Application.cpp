#include <memory>

#include "resource_manager/ResourceManager.hpp"
#include "rendering/Render.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"

#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"
#include "windowing/Window.hpp"

#include "Application.hpp"

#include <chrono>
#include <scenes/Scene_1.hpp>

using namespace Common;

void Application::Start() {
    init();
    loadResouces();

    const auto& render = Rendering::Instance();

    while(!quit) {
        Windowing::Windowing::PoolEvents();
        render->Collect(scene);
        render->Draw();

        render->SwapBuffers();
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

    auto& render = Rendering::Instance();
    render->Init(window);
}

void Application::terminate() {
    scene->Terminate();

    window.reset();
    window = nullptr;

    Rendering::Instance()->Terminate();
    Windowing::Windowing::UnSubscribe(this);
}

void Application::loadResouces() {
    scene = std::make_shared<Scenes::Scene_1>();
    scene->Init();
}