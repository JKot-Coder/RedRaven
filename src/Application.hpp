#pragma once

#include "windowing/Listener.hpp"

namespace Windowing {
    class Window;
}

class Application: public Windowing::Listener {
public:
    Application() : quit(false) {}

    void Start();
    virtual void Quit() override;
private:
    bool quit;

    std::shared_ptr<Windowing::Window> window;

    void init();
    void terminate();
};