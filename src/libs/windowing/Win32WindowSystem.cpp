#include "WindowSystem.hpp"

#include "common/Exception.hpp"
#include "common/Math.hpp"

#include "Windowing/Window.hpp"

#include <Windows.h>

namespace OpenDemo
{
    using namespace Common;

    namespace Windowing
    {
        std::vector<IListener*> WindowSystem::WindowSystem::_listeners = std::vector<IListener*>();

        WindowSystem::WindowSystem()
        {
        }

        WindowSystem::~WindowSystem()
        {
        }

        std::shared_ptr<Window> WindowSystem::Create(const WindowDescription& description)
        {
            const auto& window = std::shared_ptr<Window>(new Window());

            if (!window->Init(description))
                return nullptr;

            return window;
        }

        void WindowSystem::PoolEvents()
        {
            MSG msg;

            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    // NOTE: While GLFW does not itself post WM_QUIT, other processes
                    //       may post it to this one, for example Task Manager
                    // HACK: Treat WM_QUIT as a close on all windows

                 /*   window = _glfw.windowListHead;
                    while (window)
                    {
                        _glfwInputWindowCloseRequest(window);
                        window = window->next;
                    }*/
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }

            /* glfwPollEvents();


           SDL_Event e;

            while (SDL_PollEvent(&e))
            {
                for (auto& listener : _listeners)
                {
                    switch (e.type)
                    {
                    case SDL_QUIT:
                        listener->OnQuit();
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                    {
                        const auto& keyboardEvent = e.key;
                        auto* sdlWindow = SDL_GetWindowFromID(keyboardEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (keyboardEvent.state)
                        {
                        case SDL_PRESSED:
                            listener->OnKeyDown(*window, keyboardEvent.keysym);
                            break;
                        case SDL_RELEASED:
                            listener->OnKeyUp(*window, keyboardEvent.keysym);
                            break;
                        }

                        break;
                    }
                    case SDL_MOUSEMOTION:
                    {
                        const auto& motionEvent = e.motion;
                        auto* sdlWindow = SDL_GetWindowFromID(motionEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        listener->OnMouseMotion(*window,
                                                Vector2i(motionEvent.x, motionEvent.y),
                                                Vector2i(motionEvent.xrel, motionEvent.yrel));
                    }
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    {
                        const auto& buttonEvent = e.button;
                        auto* sdlWindow = SDL_GetWindowFromID(buttonEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (buttonEvent.state)
                        {
                        case SDL_PRESSED:
                            listener->OnMouseButtonDown(*window, buttonEvent.button);
                            break;
                        case SDL_RELEASED:
                            listener->OnMouseButtonUp(*window, buttonEvent.button);
                            break;
                        }
                    }
                    case SDL_WINDOWEVENT:
                    {
                        auto* sdlWindow = SDL_GetWindowFromID(e.window.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (e.window.event)
                        {
                        case SDL_WINDOWEVENT_RESIZED:
                            listener->OnWindowResize(*window);
                            break;
                        case SDL_WINDOWEVENT_SHOWN:
                            listener->OnWindowShown(*window);
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            listener->OnWindowHidden(*window);
                            break;
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            listener->OnWindowFocusGained(*window);
                            break;
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            listener->OnWindowFocusLost(*window);
                            break;
                        }
                        break;
                    }
                    }
                }
            }*/
        }

        void WindowSystem::Subscribe(IListener* listener)
        {
            for (auto& item : _listeners)
            {
                if (listener == item)
                    throw Common::Exception("Error subscribe listener, listener already subscribed");
            }

            _listeners.push_back(listener);
        }

        void WindowSystem::UnSubscribe(const IListener* listener)
        {
            for (auto it = _listeners.begin(); it != _listeners.end();)
            {
                if (listener == *it)
                {
                    it = _listeners.erase(it);
                    return;
                }
                ++it;
            }
        }
    }
}