#pragma once

#include "gapi/Device.hpp"
#include "windowing/Windowing.hpp"

namespace OpenDemo
{

    namespace Windowing
    {
        class InputtingWindow;
    }

    namespace Rendering
    {
        class Mesh;
        class SceneGraph;
    }

    class Application : public Windowing::IListener
    {
    public:
        Application()
            : _quit(false)
        {
        }

        void Start();
        virtual void OnQuit() override;

    private:
        bool _quit;

        std::shared_ptr<Windowing::InputtingWindow> _window;
        std::shared_ptr<Rendering::SceneGraph> _scene;
        std::unique_ptr<Render::Device> _device;

        void init();
        void terminate();

        void loadResouces();

        void OnWindowResize(const Windowing::Window& window) override;
    };
}