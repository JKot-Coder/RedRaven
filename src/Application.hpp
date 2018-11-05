#pragma once

#include "windowing/Listener.hpp"

class Application: public Windowing::IListener {
public:
    Application() : quit(false) {}

    void Start();
    virtual void Quit() override;

private:
    bool quit;
};