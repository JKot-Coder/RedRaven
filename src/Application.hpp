#pragma once

#include "windowing/Listener.hpp"

class Application: public Windowing::Listener {
public:
    Application() : quit(false) {}

    void Start();
    virtual void Quit() override;

private:
    bool quit;
};