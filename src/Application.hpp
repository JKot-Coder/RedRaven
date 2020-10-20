#pragma once

#include "gapi/DeviceInterface.hpp"
#include "gapi/Submission.hpp"

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
            submission_.reset(new Render::Submission());
        }

        void Start();
        virtual void OnQuit() override;

    private:
        bool _quit;

        std::unique_ptr<Render::Submission> submission_;
        std::shared_ptr<Windowing::InputtingWindow> _window;
        std::shared_ptr<Rendering::SceneGraph> _scene;

        void init();
        void terminate();

        void loadResouces();

        void OnWindowResize(const Windowing::Window& window) override;
    };
}