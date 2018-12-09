#pragma once

#include "windowing/Listener.hpp"

namespace Windowing {
    class Window;
}

namespace Rendering {
    class Shader;
    class Mesh;
}

class Application: public Windowing::Listener {
public:
    Application() : quit(false) {}

    void Start();
    virtual void Quit() override;
private:
    bool quit;

    std::shared_ptr<Rendering::Mesh> sphereMesh;
    std::shared_ptr<Rendering::Shader> shader;

    std::shared_ptr<Windowing::Window> window;

    void init();
    void terminate();

    void loadResouces();
};