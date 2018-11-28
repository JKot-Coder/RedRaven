#pragma once

#include "windowing/Listener.hpp"

namespace Windowing {
    class Window;
}

namespace Render {
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

    std::shared_ptr<Render::Mesh> sphereMesh;
    std::shared_ptr<Render::Shader> shader;

    std::shared_ptr<Windowing::Window> window;

    void init();
    void terminate();

    void loadResouces();
};