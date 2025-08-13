#include "Window.hpp"

#include "ecs/Ecs.hpp"


#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif OS_APPLE
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
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

    void windowCloseCallback(GLFWwindow* glfwWindow)
    {
        GetWindowEntity(glfwWindow).Emit<Window::Close>({});
    }

    eastl::any getNativeHandle(GLFWwindow* glfwWindow)
    {
#if OS_WINDOWS
        return glfwGetWin32Window(glfwWindow);
#elif OS_APPLE
        return glfwGetCocoaWindow(glfwWindow);
#else
        return nullptr;
#endif
    }

    void InitWindow(World& world)
    {
        world.System().OnEvent<OnAppear>().With<Window>().ForEach([](Ecs::World& world, Ecs::EntityId id, Window& window) {
            Ecs::Entity windowEntity = world.GetEntity(id); // Todo remove this boilerplate

            GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "", nullptr, nullptr);

            glfwSetWindowUserPointer(glfwWindow, new GlfwHandler {windowEntity});
            glfwSetWindowCloseCallback(glfwWindow, &windowCloseCallback);

            window.glfwWindow = glfwWindow;
            window.nativeHandle = getNativeHandle(glfwWindow);
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