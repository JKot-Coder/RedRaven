#include "Application.hpp"

#include <memory>

#include "common/Time.hpp"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "rendering/Render.hpp"
#include "rendering/RenderPipeline.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"

#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"
#include "windowing/InputtingWindow.hpp"

#include "scenes/Scene_1.hpp"
#include "scenes/Scene_2.hpp"

using namespace Common;

void Application::Start() {
    init();
    loadResouces();

    const auto& input = Inputting::Instance();
    const auto& time = Time::Instance();
    const auto& render = Rendering::Instance();
    auto* renderPipeline = new Rendering::RenderPipeline(window);
    renderPipeline->Init();

    time->Init();

    while(!quit) {
        Windowing::Windowing::PoolEvents();
        renderPipeline->Collect(scene);
        renderPipeline->Draw();

        scene->Update();

        render->SwapBuffers();
        input->Update();
        time->Update();
    }

    delete renderPipeline;

    terminate();
}

void Application::OnQuit() {
    quit = true;
}

void Application::init() {
    Windowing::WindowSettings settings;
    Windowing::WindowRect rect(Windowing::WindowRect::WINDOW_POSITION_CENTERED, 
		Windowing::WindowRect::WINDOW_POSITION_CENTERED, 800, 600);

    settings.Title = "OpenDemo";
    settings.WindowRect = rect;

    Windowing::Windowing::Subscribe(this);
    window = std::shared_ptr<Windowing::InputtingWindow>(new Windowing::InputtingWindow());
	window->Init(settings, true);

    Inputting::Instance()->Init();
	Inputting::Instance()->SubscribeToWindow(window);

    auto& render = Rendering::Instance();
    render->Init(window);
}

void Application::terminate() {
    scene->Terminate();

    window.reset();
    window = nullptr;

	Inputting::Instance()->Terminate();

    Rendering::Instance()->Terminate();
    Windowing::Windowing::UnSubscribe(this);
}

void Application::loadResouces() {
    scene = std::make_shared<Scenes::Scene_2>();
    scene->Init();
}