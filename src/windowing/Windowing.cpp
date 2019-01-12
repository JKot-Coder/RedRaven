#include <SDL.h>
#include <SDL_syswm.h>

#include "common/Exception.hpp"

#include "windowing/Window.hpp"
#include "windowing/Windowing.hpp"

namespace Windowing {

    std::unique_ptr<Windowing> Windowing::Windowing::windowingInstance = std::unique_ptr<Windowing>(new Windowing());
    std::vector<Windowing::Listener*> Windowing::Windowing::listeners = std::vector<Windowing::Listener*>();

    Windowing::Windowing(){
        SDL_SetRelativeMouseMode(SDL_TRUE);

        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            throw Common::Exception("Could not initialize SDL video subsystem (%s)", SDL_GetError());
    }

    Windowing::~Windowing(){
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SDL_Quit();
    }

    void Windowing::PoolEvents() {
        SDL_Event e;

        while (SDL_PollEvent(&e)){
            for(auto& listener: listeners) {
                switch (e.type) {
                    case SDL_QUIT:
                        listener->Quit();
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP: {
                        SDL_KeyboardEvent &keyboardEvent = e.key;

                        switch (keyboardEvent.state){
                            case SDL_PRESSED:
                                listener->KeyDown(keyboardEvent.keysym);
                                break;
                            case SDL_RELEASED:
                                listener->KeyUp(keyboardEvent.keysym);
                                break;
                        }

                        break;
                    }
                    case SDL_MOUSEMOTION: {
                        listener->MouseMotion(e.motion);
                    }
                    case SDL_WINDOWEVENT: {
                        SDL_Window *sdlWindow = SDL_GetWindowFromID(e.window.windowID);
                        Window *window = static_cast<Window *>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (e.window.event) {
                            case SDL_WINDOWEVENT_RESIZED:
                                listener->WindowResize(*window);
                                break;
                        }
                        break;
                    }
                }
            }
        }
    }

    const std::shared_ptr<Window> Windowing::CreateWindow(const WindowSettings &settings){
        auto window = new Window();
        window->Init(settings);

        return std::shared_ptr<Window>(window);
    }

    void Windowing::Subscribe(Listener *listener) {
        for(auto& item: listeners) {
            if(listener == item)
                throw Common::Exception("Error subscribe listener, listener already subscribed");
        }

        listeners.push_back(listener);
    }

    void Windowing::UnSubscribe(const Listener* listener) {
        for (auto it = listeners.begin(); it != listeners.end(); ) {
            if (listener == *it) {
                it = listeners.erase(it);
                return;
            }
            ++it;
        }
    }

}