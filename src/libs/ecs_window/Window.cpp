#include "Window.hpp"

#include "ecs/Ecs.hpp"
#include <GLFW/glfw3.h>
namespace RR::Ecs::WindowModule
{
    struct Glfw
    {
        ECS_SINGLETON;
    };

    struct GlfwHandler
    {
        Ecs::Entity windowEntity;
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

    Ecs::Entity GetWindowEntity(GLFWwindow* glfwWindow)
    {
        const auto handler = static_cast<GlfwHandler*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(handler);

        return handler->windowEntity;
    }
    void InitWindow(World& world)
    {
        world.System().OnEvent<OnAppear>().With<Window>().ForEach([](Ecs::World& world, Ecs::EntityId id, Window& window) {
            Ecs::Entity windowEntity = world.GetEntity(id); // Todo remove this boilerplate

            GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "", nullptr, nullptr);

            glfwSetWindowUserPointer(glfwWindow, new GlfwHandler {windowEntity});
            window.glfwWindow = glfwWindow;
        });

        world.System().OnEvent<OnDissapear>().With<Window>().ForEach([](Window& window) {
            const auto handler = static_cast<GlfwHandler*>(glfwGetWindowUserPointer(window.glfwWindow));
            ASSERT(handler);

            delete handler;

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