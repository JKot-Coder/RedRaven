#pragma once

#include "platform/Window.hpp"

#include "common/Event.hpp"
#include "common/Singleton.hpp"

namespace RR
{
    namespace Platform
    {
        struct VideoMode
        {
            Vector2i resolution;
            uint32_t redBits;
            uint32_t greenBits;
            uint32_t blueBits;
            uint32_t refreshRate;
        };

        struct Monitor
        {
            enum class Event
            {
                Disconnected,
                Connected
            };

            Vector2i position;
            VideoMode videoMode;
            Rect2i workArea;
            Vector2 dpiScale;
        };

        class Toolkit final : public Singleton<Toolkit>
        {
        public:
            Toolkit() = default;
            ~Toolkit();

            void Init();

            Event<void(const Monitor& monitor, Monitor::Event event)> OnMonitorConfigChanged;
            std::vector<Monitor> GetMonitors() const;

            void PoolEvents() const;

            // CreatePlatformWindow to avoid name collision with CreateWindow define from Windows.h
            std::shared_ptr<Window> CreatePlatformWindow(const Window::Description& description) const;

        private:
            bool isInited_ = false;
        };
    }
}