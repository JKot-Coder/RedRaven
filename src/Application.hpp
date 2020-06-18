#pragma once

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
            : quit(false)
        {
        }

        void Start();
        virtual void OnQuit() override;

    private:
        bool quit;

        std::shared_ptr<Windowing::InputtingWindow> window;
        std::shared_ptr<Rendering::SceneGraph> scene;

        void init();
        void terminate();

        void loadResouces();
    };
}