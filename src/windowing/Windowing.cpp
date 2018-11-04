#include <SDL.h>
#include <SDL_syswm.h>

#include "common/Exception.hpp"
#include "windowing/Window.hpp"
#include "windowing/Windowing.hpp"

namespace Windowing {

    std::unique_ptr<Windowing> Windowing::Windowing::windowingInstance = std::unique_ptr<Windowing>(new Windowing());

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
        bool quit;

        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                quit = true;
            }
        }
    }

    const Window& Windowing::CreateWindow(const WindowSettings &settings){
        Window window;
        window.Init(settings);

        return window;
    }

}