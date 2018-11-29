#include <memory>

#include "resource_manager/ResourceManager.hpp"
#include "render/Rendering.hpp"
#include "render/Primitives.hpp"
#include "render/Shader.hpp"
#include "render/Mesh.hpp"

#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"
#include "windowing/Window.hpp"

#include "Application.hpp"

using namespace Common;

void Application::Start() {
    init();
    loadResouces();

    const auto& render = Render::Instance();

    while(!quit) {
        Windowing::Windowing::PoolEvents();
        render->Update();

        render->SetClearColor(vec4(0.25, 0.25, 0.25, 0));
        render->Clear(true, true);

        vec3 eyePos = vec3(0, 0, -7);
        vec3 targetPos = vec3(0, 0, 0);

        vec2 windowSize = window->GetSize();

        mat4 proj(mat4::PROJ_ZERO_POS, 90, windowSize.x / windowSize.y, 1, 100);
        mat4 viewInv(eyePos, targetPos, vec3(0, -1, 0));
        mat4 view = viewInv.inverseOrtho();
        mat4 viewProj = proj * view;

        mat4 model;
        model.identity();

        shader->Bind();
        shader->SetParam(Render::Shader::VIEW_PROJECTION_MATRIX, viewProj, 1);
        shader->SetParam(Render::Shader::MODEL_MATRIX, model, 1);

        sphereMesh->Draw();

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

    auto& render = Render::Instance();
    render->Init(window);

}

void Application::terminate() {
    window.reset();
    window = nullptr;

    Render::Instance()->Terminate();
    Windowing::Windowing::UnSubscribe(this);
}

void Application::loadResouces() {
   auto *resourceManager = ResourceManager::Instance().get();
   shader = resourceManager->LoadShader("resources/test.shader");

   sphereMesh = Render::Primitives::GetSphereMesh(23);
}


