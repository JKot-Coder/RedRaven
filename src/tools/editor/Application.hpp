#pragma once

#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "common/Math.hpp"

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
        void resizeCallback(const Platform::Window&, const Common::Vector2i& size);

        // Our state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImguiPlatfomImpl imguiPlatformInput;
    };
}