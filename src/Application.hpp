#pragma once

#include "windowing/Windowing.hpp"

namespace Windowing {
    class Window;
}

namespace Rendering {
    class Mesh;
    class SceneGraph;
}

class Application: public Windowing::Windowing::Listener {
public:
    Application() : quit(false) {}

    void Start();
    virtual void Quit() override;
private:
    bool quit;

    std::shared_ptr<Windowing::Window> window;
    std::shared_ptr<Rendering::SceneGraph> scene;

    void init();
    void terminate();

    void loadResouces();
};