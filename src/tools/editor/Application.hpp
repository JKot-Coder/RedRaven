#pragma once

//#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "math/ForwardDeclarations.hpp"
#include "ecs/World.hpp"
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
    //    void resizeCallback(const Platform::Window&, const Vector2i& size);

        // Our state
       // ImguiPlatfomImpl imguiPlatformInput;
        Ecs::World editorWorld;
        std::unique_ptr<EcsModule::Manager> ecsManager;
    };
}