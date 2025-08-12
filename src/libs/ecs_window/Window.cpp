#include "Window.hpp"

#include "ecs/Ecs.hpp"
#include <GLFW/glfw3.h>
namespace RR::Ecs
{
    void InitWindowModule(World& world)
    {
        world.System().OnEvent<OnAppear>().With<Window>().ForEach([](Window& window) {
            window.window_ = glfwCreateWindow(100, 100, "Hello", nullptr, nullptr);
        });

        world.System().OnEvent<OnDissapear>().With<Window>().ForEach([](Window& window) {
            glfwDestroyWindow(window.window_);
        });
    }
}