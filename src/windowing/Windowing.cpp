#include <SDL.h>
#include <SDL_syswm.h>

#include "common/Exception.hpp"

#include "windowing/Window.hpp"
#include "windowing/Windowing.hpp"
#include "windowing/Listener.hpp"

namespace Windowing {

    std::unique_ptr<Windowing> Windowing::Windowing::windowingInstance = std::unique_ptr<Windowing>(new Windowing());
    std::vector<std::shared_ptr<IListener>> Windowing::Windowing::listeners = std::vector<std::shared_ptr<IListener>>();

    Windowing::Windowing(){
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
                if (e.type == SDL_QUIT) {
                    listener.get()->Quit();
                }
            }

        }
    }

    const Window& Windowing::CreateWindow(const WindowSettings &settings){
        Window window;
        window.Init(settings);

        return window;
    }

    void Windowing::Subscribe(const std::shared_ptr<IListener> &listener) {
        for(auto& item: listeners) {
            if(listener.get() == item.get())
                throw Common::Exception("Error subscribe listener, listener already subscribed");
        }

        listeners.push_back(listener);
    }

    void Windowing::UnSubscribe(const std::shared_ptr<IListener> &listener) {
        for (auto it = listeners.begin(); it != listeners.end(); ) {
            if (listener.get() == (*it).get()) {
                it = listeners.erase(it);
                return;
            }
            ++it;
        }
    }

}