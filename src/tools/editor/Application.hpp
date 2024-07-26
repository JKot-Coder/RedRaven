#pragma once

#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "math/ForwardDeclarations.hpp"
#include "flecs.h"

namespace RR
{
    namespace Render
    {
        class DeviceContext;
    }

    class Application final
    {
    public:
        int Run();

    private:
        void init();
        void draw(Render::DeviceContext& deviceContext, float dt);
        void resizeCallback(const Platform::Window&, const Vector2i& size);

        // Our state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImguiPlatfomImpl imguiPlatformInput;
        flecs::world world;
    };
}