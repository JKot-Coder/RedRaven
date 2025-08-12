#include "Window.hpp"

#include "ecs/Ecs.hpp"
#include <GLFW/glfw3.h>
namespace RR::Ecs::WindowModule
{

    struct Glfw
    {
        ECS_SINGLETON;
    };

    void InitGlfw(World& world)
    {
        world.System().OnEvent<OnAppear>().With<Glfw>().ForEach([]() {
            glfwInit();
        });

        world.System().OnEvent<OnDissapear>().With<Glfw>().ForEach([]() {
            glfwTerminate();
        });

        world.System().OnEvent<Tick>().With<Glfw>().ForEach([]() {
            glfwPollEvents();
        });
    }

    void InitWindow(World& world)
    {
        world.System().OnEvent<OnAppear>().With<Window>().ForEach([](Window& window) {
            GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "", nullptr, nullptr);
            window.glfwWindow = glfwWindow;
        });

        world.System().OnEvent<OnDissapear>().With<Window>().ForEach([](Window& window) {
            glfwDestroyWindow(window.glfwWindow);
        });
    }

    void Init(World& world)
    {
        InitGlfw(world);
        InitWindow(world);

        world.OrderSystems();
        world.Entity().Add<Glfw>().Apply();
    }
}