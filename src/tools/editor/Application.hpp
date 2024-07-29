#pragma once

#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "math/ForwardDeclarations.hpp"
#include "flecs.h"
#include "ecs_module/Manager.hpp"

namespace RR
{
    namespace Render
    {
        class DeviceContext;
    }

    class Application final
    {
    public:
        Application();
        ~Application();
        int Run();

    private:
        void init();
        void draw(Render::DeviceContext& deviceContext, float dt);
        void resizeCallback(const Platform::Window&, const Vector2i& size);

        // Our state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImguiPlatfomImpl imguiPlatformInput;
        flecs::world editorWorld;
        std::unique_ptr<EcsModule::Manager> ecsManager;
    };
}