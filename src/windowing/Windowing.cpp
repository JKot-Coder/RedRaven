#include "Windowing.hpp"

#include <SDL.h>
#include <SDL_syswm.h>

#include "common/VecMath.h"
#include "common/Exception.hpp"

#include "windowing/Window.hpp"

using namespace Common;

namespace Windowing {

    std::unique_ptr<Windowing> Windowing::Windowing::windowingInstance = std::unique_ptr<Windowing>(new Windowing());
    std::vector<IListener*> Windowing::Windowing::listeners = std::vector<IListener*>();

    Windowing::Windowing() {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            throw Common::Exception("Could not initialize SDL video subsystem (%s)", SDL_GetError());
    }

    Windowing::~Windowing() {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SDL_Quit();
    }

    void Windowing::PoolEvents() {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            for(auto& listener: listeners) {
                switch (e.type) {
                    case SDL_QUIT:
                        listener->OnQuit();
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP: {
                        const auto &keyboardEvent = e.key;
						auto *sdlWindow = SDL_GetWindowFromID(keyboardEvent.windowID);
						const auto *window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (keyboardEvent.state) {
                            case SDL_PRESSED:
                                listener->OnKeyDown(*window, keyboardEvent.keysym);
                                break;
                            case SDL_RELEASED:
                                listener->OnKeyUp(*window, keyboardEvent.keysym);
                                break;
                        }

                        break;
                    }
                    case SDL_MOUSEMOTION: {
						const auto &motionEvent = e.motion;
						auto *sdlWindow = SDL_GetWindowFromID(motionEvent.windowID);
						const auto *window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));
					
                        listener->OnMouseMotion(*window, 
							vec2(static_cast<float>(motionEvent.x), static_cast<float>(motionEvent.y)),
							vec2(static_cast<float>(motionEvent.xrel), static_cast<float>(motionEvent.yrel)));
                    }
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP: {
						const auto &buttonEvent = e.button;
						auto *sdlWindow = SDL_GetWindowFromID(buttonEvent.windowID);
						const auto *window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

						switch (buttonEvent.state) {
						case SDL_PRESSED:
							listener->OnMouseButtonDown(*window, buttonEvent.button);
							break;
						case SDL_RELEASED:
							listener->OnMouseButtonUp(*window, buttonEvent.button);
							break;
						}
					}
                    case SDL_WINDOWEVENT: {
						auto *sdlWindow = SDL_GetWindowFromID(e.window.windowID);
						const auto *window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (e.window.event) {
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
        }
    }

    void Windowing::Subscribe(IListener *listener) {
        for(auto& item: listeners) {
            if(listener == item)
                throw Common::Exception("Error subscribe listener, listener already subscribed");
        }

        listeners.push_back(listener);
    }

    void Windowing::UnSubscribe(const IListener* listener) {
        for (auto it = listeners.begin(); it != listeners.end(); ) {
            if (listener == *it) {
                it = listeners.erase(it);
                return;
            }
            ++it;
        }
    }

}