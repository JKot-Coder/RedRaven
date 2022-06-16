#pragma once

namespace RR
{
    namespace Platform
    {
        class Window;
    }

    class ImguiPlatfomImpl
    {
    public:
        bool Init(const std::shared_ptr<Platform::Window>& window, bool install_callbacks);
        void Shutdown();
        void NewFrame(float dt);
    };
}